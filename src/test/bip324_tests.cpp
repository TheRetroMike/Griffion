// Copyright (c) 2023 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bip324.h>
#include <chainparams.h>
#include <key.h>
#include <pubkey.h>
#include <span.h>
#include <test/util/random.h>
#include <test/util/setup_common.h>
#include <util/strencodings.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <boost/test/unit_test.hpp>

namespace {

void TestBIP324PacketVector(
    uint32_t in_idx,
    const std::string& in_priv_ours_hex,
    const std::string& in_ellswift_ours_hex,
    const std::string& in_ellswift_theirs_hex,
    bool in_initiating,
    const std::string& in_contents_hex,
    uint32_t in_multiply,
    const std::string& in_aad_hex,
    bool in_ignore,
    const std::string& mid_send_garbage_hex,
    const std::string& mid_recv_garbage_hex,
    const std::string& out_session_id_hex,
    const std::string& out_ciphertext_hex,
    const std::string& out_ciphertext_endswith_hex)
{
    // Convert input from hex to char/byte vectors/arrays.
    const auto in_priv_ours = ParseHex(in_priv_ours_hex);
    const auto in_ellswift_ours = ParseHex<std::byte>(in_ellswift_ours_hex);
    const auto in_ellswift_theirs = ParseHex<std::byte>(in_ellswift_theirs_hex);
    const auto in_contents = ParseHex<std::byte>(in_contents_hex);
    const auto in_aad = ParseHex<std::byte>(in_aad_hex);
    const auto mid_send_garbage = ParseHex<std::byte>(mid_send_garbage_hex);
    const auto mid_recv_garbage = ParseHex<std::byte>(mid_recv_garbage_hex);
    const auto out_session_id = ParseHex<std::byte>(out_session_id_hex);
    const auto out_ciphertext = ParseHex<std::byte>(out_ciphertext_hex);
    const auto out_ciphertext_endswith = ParseHex<std::byte>(out_ciphertext_endswith_hex);

    // Load keys
    CKey key;
    key.Set(in_priv_ours.begin(), in_priv_ours.end(), true);
    EllSwiftPubKey ellswift_ours(in_ellswift_ours);
    EllSwiftPubKey ellswift_theirs(in_ellswift_theirs);

    // Instantiate encryption BIP324 cipher.
    BIP324Cipher cipher(key, ellswift_ours);
    BOOST_CHECK(!cipher);
    BOOST_CHECK(cipher.GetOurPubKey() == ellswift_ours);
    cipher.Initialize(ellswift_theirs, in_initiating);
    BOOST_CHECK(cipher);

    // Compare session variables.
    BOOST_CHECK(Span{out_session_id} == cipher.GetSessionID());
    BOOST_CHECK(Span{mid_send_garbage} == cipher.GetSendGarbageTerminator());
    BOOST_CHECK(Span{mid_recv_garbage} == cipher.GetReceiveGarbageTerminator());

    // Vector of encrypted empty messages, encrypted in order to seek to the right position.
    std::vector<std::vector<std::byte>> dummies(in_idx);

    // Seek to the numbered packet.
    for (uint32_t i = 0; i < in_idx; ++i) {
        dummies[i].resize(cipher.EXPANSION);
        cipher.Encrypt({}, {}, true, dummies[i]);
    }

    // Construct contents and encrypt it.
    std::vector<std::byte> contents;
    for (uint32_t i = 0; i < in_multiply; ++i) {
        contents.insert(contents.end(), in_contents.begin(), in_contents.end());
    }
    std::vector<std::byte> ciphertext(contents.size() + cipher.EXPANSION);
    cipher.Encrypt(contents, in_aad, in_ignore, ciphertext);

    // Verify ciphertext. Note that the test vectors specify either out_ciphertext (for short
    // messages) or out_ciphertext_endswith (for long messages), so only check the relevant one.
    if (!out_ciphertext.empty()) {
        BOOST_CHECK(out_ciphertext == ciphertext);
    } else {
        BOOST_CHECK(ciphertext.size() >= out_ciphertext_endswith.size());
        BOOST_CHECK(Span{out_ciphertext_endswith} == Span{ciphertext}.last(out_ciphertext_endswith.size()));
    }

    for (unsigned error = 0; error <= 12; ++error) {
        // error selects a type of error introduced:
        // - error=0: no errors, decryption should be successful
        // - error=1: wrong side
        // - error=2..9: bit error in ciphertext
        // - error=10: bit error in aad
        // - error=11: extra 0x00 at end of aad
        // - error=12: message index wrong

        // Instantiate self-decrypting BIP324 cipher.
        BIP324Cipher dec_cipher(key, ellswift_ours);
        BOOST_CHECK(!dec_cipher);
        BOOST_CHECK(dec_cipher.GetOurPubKey() == ellswift_ours);
        dec_cipher.Initialize(ellswift_theirs, (error == 1) ^ in_initiating, /*self_decrypt=*/true);
        BOOST_CHECK(dec_cipher);

        // Compare session variables.
        BOOST_CHECK((Span{out_session_id} == dec_cipher.GetSessionID()) == (error != 1));
        BOOST_CHECK((Span{mid_send_garbage} == dec_cipher.GetSendGarbageTerminator()) == (error != 1));
        BOOST_CHECK((Span{mid_recv_garbage} == dec_cipher.GetReceiveGarbageTerminator()) == (error != 1));

        // Seek to the numbered packet.
        if (in_idx == 0 && error == 12) continue;
        uint32_t dec_idx = in_idx ^ (error == 12 ? (1U << InsecureRandRange(16)) : 0);
        for (uint32_t i = 0; i < dec_idx; ++i) {
            unsigned use_idx = i < in_idx ? i : 0;
            bool dec_ignore{false};
            dec_cipher.DecryptLength(Span{dummies[use_idx]}.first(cipher.LENGTH_LEN));
            dec_cipher.Decrypt(Span{dummies[use_idx]}.subspan(cipher.LENGTH_LEN), {}, dec_ignore, {});
        }

        // Construct copied (and possibly damaged) copy of ciphertext.
        // Decrypt length
        auto to_decrypt = ciphertext;
        if (error >= 2 && error <= 9) {
            to_decrypt[InsecureRandRange(to_decrypt.size())] ^= std::byte(1U << (error - 2));
        }

        // Decrypt length and resize ciphertext to accommodate.
        uint32_t dec_len = dec_cipher.DecryptLength(MakeByteSpan(to_decrypt).first(cipher.LENGTH_LEN));
        to_decrypt.resize(dec_len + cipher.EXPANSION);

        // Construct copied (and possibly damaged) copy of aad.
        auto dec_aad = in_aad;
        if (error == 10) {
            if (in_aad.size() == 0) continue;
            dec_aad[InsecureRandRange(dec_aad.size())] ^= std::byte(1U << InsecureRandRange(8));
        }
        if (error == 11) dec_aad.push_back({});

        // Decrypt contents.
        std::vector<std::byte> decrypted(dec_len);
        bool dec_ignore{false};
        bool dec_ok = dec_cipher.Decrypt(Span{to_decrypt}.subspan(cipher.LENGTH_LEN), dec_aad, dec_ignore, decrypted);

        // Verify result.
        BOOST_CHECK(dec_ok == !error);
        if (dec_ok) {
            BOOST_CHECK(decrypted == contents);
            BOOST_CHECK(dec_ignore == in_ignore);
        }
    }
}

}  // namespace

BOOST_FIXTURE_TEST_SUITE(bip324_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(packet_test_vectors) {
    // BIP324 key derivation uses network magic in the HKDF process. We use mainnet params here
    // as that is what the test vectors are written for.
    SelectParams(ChainType::MAIN);

    // The test vectors are converted using the following Python code in the BIP bip-0324/ directory:
    //
    // import sys
    // import csv
    // with open('packet_encoding_test_vectors.csv', newline='', encoding='utf-8') as csvfile:
    //     reader = csv.DictReader(csvfile)
    //     quote = lambda x: "\"" + x + "\""
    //     for row in reader:
    //         args = [
    //             row['in_idx'],
    //             quote(row['in_priv_ours']),
    //             quote(row['in_ellswift_ours']),
    //             quote(row['in_ellswift_theirs']),
    //             "true" if int(row['in_initiating']) else "false",
    //             quote(row['in_contents']),
    //             row['in_multiply'],
    //             quote(row['in_aad']),
    //             "true" if int(row['in_ignore']) else "false",
    //             quote(row['mid_send_garbage_terminator']),
    //             quote(row['mid_recv_garbage_terminator']),
    //             quote(row['out_session_id']),
    //             quote(row['out_ciphertext']),
    //             quote(row['out_ciphertext_endswith'])
    //         ]
    //         print("    TestBIP324PacketVector(\n        " + ",\n        ".join(args) + ");")
    TestBIP324PacketVector(
        1,
        "61062ea5071d800bbfd59e2e8b53d47d194b095ae5a4df04936b49772ef0d4d7",
        "ec0adff257bbfe500c188c80b4fdd640f6b45a482bbc15fc7cef5931deff0aa186f6eb9bba7b85dc4dcc28b28722de1e3d9108b985e2967045668f66098e475b",
        "a4a94dfce69b4a2a0a099313d10f9f7e7d649d60501c9e1d274c300e0d89aafaffffffffffffffffffffffffffffffffffffffffffffffffffffffff8faf88d5",
        true,
        "8e",
        1,
        "",
        false,
        "8f980434ae8b1254ef9b435fa3447dad",
        "8c758f18aa74d5baa338b3888a366bb9",
        "2242e45f95c28d09ff0066c85c83c1204eb70fcb5e9fb3912f74e6271ac2ecff",
        "c663ae567a2c83335f997583a231791eb0f2365f2a",
        "");
    TestBIP324PacketVector(
        999,
        "1f9c581b35231838f0f17cf0c979835baccb7f3abbbb96ffcc318ab71e6e126f",
        "a1855e10e94e00baa23041d916e259f7044e491da6171269694763f018c7e63693d29575dcb464ac816baa1be353ba12e3876cba7628bd0bd8e755e721eb0140",
        "fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f0000000000000000000000000000000000000000000000000000000000000000",
        false,
        "3eb1d4e98035cfd8eeb29bac969ed3824a",
        1,
        "",
        false,
        "6ea131c36abfffc50c1f59854245ce59",
        "720d3ddc287c1799b08a434634a4d336",
        "7ea63b1f0e90aba5cf2382ca65f391d082c9268d87b66b877465e2d1a99cec5d",
        "694ba5a433a5e4a0ba9159e3b1cc9842244d8e3c1d056b3244a867fe1cf544a12f8c3a5c79",
        "");
    TestBIP324PacketVector(
        0,
        "0286c41cd30913db0fdff7a64ebda5c8e3e7cef10f2aebc00a7650443cf4c60d",
        "d1ee8a93a01130cbf299249a258f94feb5f469e7d0f2f28f69ee5e9aa8f9b54a60f2c3ff2d023634ec7f4127a96cc11662e402894cf1f694fb9a7eaa5f1d9244",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff22d5e441524d571a52b3def126189d3f416890a99d4da6ede2b0cde1760ce2c3f98457ae",
        true,
        "054290a6c6ba8d80478172e89d32bf690913ae9835de6dcf206ff1f4d652286fe0ddf74deba41d55de3edc77c42a32af79bbea2c00bae7492264c60866ae5a",
        1,
        "84932a55aac22b51e7b128d31d9f0550da28e6a3f394224707d878603386b2f9d0c6bcd8046679bfed7b68c517e7431e75d9dd34605727d2ef1c2babbf680ecc8d68d2c4886e9953a4034abde6da4189cd47c6bb3192242cf714d502ca6103ee84e08bc2ca4fd370d5ad4e7d06c7fbf496c6c7cc7eb19c40c61fb33df2a9ba48497a96c98d7b10c1f91098a6b7b16b4bab9687f27585ade1491ae0dba6a79e1e2d85dd9d9d45c5135ca5fca3f0f99a60ea39edbc9efc7923111c937913f225d67788d5f7e8852b697e26b92ec7bfcaa334a1665511c2b4c0a42d06f7ab98a9719516c8fd17f73804555ee84ab3b7d1762f6096b778d3cb9c799cbd49a9e4a325197b4e6cc4a5c4651f8b41ff88a92ec428354531f970263b467c77ed11312e2617d0d53fe9a8707f51f9f57a77bfb49afe3d89d85ec05ee17b9186f360c94ab8bb2926b65ca99dae1d6ee1af96cad09de70b6767e949023e4b380e66669914a741ed0fa420a48dbc7bfae5ef2019af36d1022283dd90655f25eec7151d471265d22a6d3f91dc700ba749bb67c0fe4bc0888593fbaf59d3c6fff1bf756a125910a63b9682b597c20f560ecb99c11a92c8c8c3f7fbfaa103146083a0ccaecf7a5f5e735a784a8820155914a289d57d8141870ffcaf588882332e0bcd8779efa931aa108dab6c3cce76691e345df4a91a03b71074d66333fd3591bff071ea099360f787bbe43b7b3dff2a59c41c7642eb79870222ad1c6f2e5a191ed5acea51134679587c9cf71c7d8ee290be6bf465c4ee47897a125708704ad610d8d00252d01959209d7cd04d5ecbbb1419a7e84037a55fefa13dee464b48a35c96bcb9a53e7ed461c3a1607ee00c3c302fd47cd73fda7493e947c9834a92d63dcfbd65aa7c38c3e3a2748bb5d9a58e7495d243d6b741078c8f7ee9c8813e473a323375702702b0afae1550c8341eedf5247627343a95240cb02e3e17d5dca16f8d8d3b2228e19c06399f8ec5c5e9dbe4caef6a0ea3ffb1d3c7eac03ae030e791fa12e537c80d56b55b764cadf27a8701052df1282ba8b5e3eb62b5dc7973ac40160e00722fa958d95102fc25c549d8c0e84bed95b7acb61ba65700c4de4feebf78d13b9682c52e937d23026fb4c6193e6644e2d3c99f91f4f39a8b9fc6d013f89c3793ef703987954dc0412b550652c01d922f525704d32d70d6d4079bc3551b563fb29577b3aecdc9505011701dddfd94830431e7a4918927ee44fb3831ce8c4513839e2deea1287f3fa1ab9b61a256c09637dbc7b4f0f8fbb783840f9c24526da883b0df0c473cf231656bd7bc1aaba7f321fec0971c8c2c3444bff2f55e1df7fea66ec3e440a612db9aa87bb505163a59e06b96d46f50d8120b92814ac5ab146bc78dbbf91065af26107815678ce6e33812e6bf3285d4ef3b7b04b076f21e7820dcbfdb4ad5218cf4ff6a65812d8fcb98ecc1e95e2fa58e3efe4ce26cd0bd400d6036ab2ad4f6c713082b5e3f1e04eb9e3b6c8f63f57953894b9e220e0130308e1fd91f72d398c1e7962ca2c31be83f31d6157633581a0a6910496de8d55d3d07090b6aa087159e388b7e7dec60f5d8a60d93ca2ae91296bd484d916bfaaa17c8f45ea4b1a91b37c82821199a2b7596672c37156d8701e7352aa48671d3b1bbbd2bd5f0a2268894a25b0cb2514af39c8743f8cce8ab4b523053739fd8a522222a09acf51ac704489cf17e4b7125455cb8f125b4d31af1eba1f8cf7f81a5a100a141a7ee72e8083e065616649c241f233645c5fc865d17f0285f5c52d9f45312c979bfb3ce5f2a1b951deddf280ffb3f370410cffd1583bfa90077835aa201a0712d1dcd1293ee177738b14e6b5e2a496d05220c3253bb6578d6aff774be91946a614dd7e879fb3dcf7451e0b9adb6a8c44f53c2c464bcc0019e9fad89cac7791a0a3f2974f759a9856351d4d2d7c5612c17cfc50f8479945df57716767b120a590f4bf656f4645029a525694d8a238446c5f5c2c1c995c09c1405b8b1eb9e0352ffdf766cc964f8dcf9f8f043dfab6d102cf4b298021abd78f1d9025fa1f8e1d710b38d9d1652f2d88d1305874ec41609b6617b65c5adb19b6295dc5c5da5fdf69f28144ea12f17c3c6fcce6b9b5157b3dfc969d6725fa5b098a4d9b1d31547ed4c9187452d281d0a5d456008caf1aa251fac8f950ca561982dc2dc908d3691ee3b6ad3ae3d22d002577264ca8e49c523bd51c4846be0d198ad9407bf6f7b82c79893eb2c05fe9981f687a97a4f01fe45ff8c8b7ecc551135cd960a0d6001ad35020be07ffb53cb9e731522ca8ae9364628914b9b8e8cc2f37f03393263603cc2b45295767eb0aac29b0930390eb89587ab2779d2e3decb8042acece725ba42eda650863f418f8d0d50d104e44fbbe5aa7389a4a144a8cecf00f45fb14c39112f9bfb56c0acbd44fa3ff261f5ce4acaa5134c2c1d0cca447040820c81ab1bcdc16aa075b7c68b10d06bbb7ce08b5b805e0238f24402cf24a4b4e00701935a0c68add3de090903f9b85b153cb179a582f57113bfc21c2093803f0cfa4d9d4672c2b05a24f7e4c34a8e9101b70303a7378b9c50b6cddd46814ef7fd73ef6923feceab8fc5aa8b0d185f2e83c7a99dcb1077c0ab5c1f5d5f01ba2f0420443f75c4417db9ebf1665efbb33dca224989920a64b44dc26f682cc77b4632c8454d49135e52503da855bc0f6ff8edc1145451a9772c06891f41064036b66c3119a0fc6e80dffeb65dc456108b7ca0296f4175fff3ed2b0f842cd46bd7e86f4c62dfaf1ddbf836263c00b34803de164983d0811cebfac86e7720c726d3048934c36c23189b02386a722ca9f0fe00233ab50db928d3bccea355cc681144b8b7edcaae4884d5a8f04425c0890ae2c74326e138066d8c05f4c82b29df99b034ea727afde590a1f2177ace3af99cfb1729d6539ce7f7f7314b046aab74497e63dd399e1f7d5f16517c23bd830d1fdee810f3c3b77573dd69c4b97d80d71fb5a632e00acdfa4f8e829faf3580d6a72c40b28a82172f8dcd4627663ebf6069736f21735fd84a226f427cd06bb055f94e7c92f31c48075a2955d82a5b9d2d0198ce0d4e131a112570a8ee40fb80462a81436a58e7db4e34b6e2c422e82f934ecda9949893da5730fc5c23c7c920f363f85ab28cc6a4206713c3152669b47efa8238fa826735f17b4e78750276162024ec85458cd5808e06f40dd9fd43775a456a3ff6cae90550d76d8b2899e0762ad9a371482b3e38083b1274708301d6346c22fea9bb4b73db490ff3ab05b2f7f9e187adef139a7794454b7300b8cc64d3ad76c0e4bc54e08833a4419251550655380d675bc91855aeb82585220bb97f03e976579c08f321b5f8f70988d3061f41465517d53ac571dbf1b24b94443d2e9a8e8a79b392b3d6a4ecdd7f626925c365ef6221305105ce9b5f5b6ecc5bed3d702bd4b7f5008aa8eb8c7aa3ade8ecf6251516fbefeea4e1082aa0e1848eddb31ffe44b04792d296054402826e4bd054e671f223e5557e4c94f89ca01c25c44f1a2ff2c05a70b43408250705e1b858bf0670679fdcd379203e36be3500dd981b1a6422c3cf15224f7fefdef0a5f225c5a09d15767598ecd9e262460bb33a4b5d09a64591efabc57c923d3be406979032ae0bc0997b65336a06dd75b253332ad6a8b63ef043f780a1b3fb6d0b6cad98b1ef4a02535eb39e14a866cfc5fc3a9c5deb2261300d71280ebe66a0776a151469551c3c5fa308757f956655278ec6330ae9e3625468c5f87e02cd9a6489910d4143c1f4ee13aa21a6859d907b788e28572fecee273d44e4a900fa0aa668dd861a60fb6b6b12c2c5ef3c8df1bd7ef5d4b0d1cdb8c15fffbb365b9784bd94abd001c6966216b9b67554ad7cb7f958b70092514f7800fc40244003e0fd1133a9b850fb17f4fcafde07fc87b07fb510670654a5d2d6fc9876ac74728ea41593beef003d6858786a52d3a40af7529596767c17000bfaf8dc52e871359f4ad8bf6e7b2853e5229bdf39657e213580294a5317c5df172865e1e17fe37093b585e04613f5f078f761b2b1752eb32983afda24b523af8851df9a02b37e77f543f18888a782a994a50563334282bf9cdfccc183fdf4fcd75ad86ee0d94f91ee2300a5befbccd14e03a77fc031a8cfe4f01e4c5290f5ac1da0d58ea054bd4837cfd93e5e34fc0eb16e48044ba76131f228d16cde9b0bb978ca7cdcd10653c358bdb26fdb723a530232c32ae0a4cecc06082f46e1c1d596bfe60621ad1e354e01e07b040cc7347c016653f44d926d13ca74e6cbc9d4ab4c99f4491c95c76fff5076b3936eb9d0a286b97c035ca88a3c6309f5febfd4cdaac869e4f58ed409b1e9eb4192fb2f9c2f12176d460fd98286c9d6df84598f260119fd29c63f800c07d8df83d5cc95f8c2fea2812e7890e8a0718bb1e031ecbebc0436dcf3e3b9a58bcc06b4c17f711f80fe1dffc3326a6eb6e00283055c6dabe20d311bfd5019591b7954f8163c9afad9ef8390a38f3582e0a79cdf0353de8eeb6b5f9f27b16ffdef7dd62869b4840ee226ccdce95e02c4545eb981b60571cd83f03dc5eaf8c97a0829a4318a9b3dc06c0e003db700b2260ff1fa8fee66890e637b109abb03ec901b05ca599775f48af50154c0e67d82bf0f558d7d3e0778dc38bea1eb5f74dc8d7f90abdf5511a424be66bf8b6a3cacb477d2e7ef4db68d2eba4d5289122d851f9501ba7e9c4957d8eba3be3fc8e785c4265a1d65c46f2809b70846c693864b169c9dcb78be26ea14b8613f145b01887222979a9e67aee5f800caa6f5c4229bdeefc901232ace6143c9865e4d9c07f51aa200afaf7e48a7d1d8faf366023beab12906ffcb3eaf72c0eb68075e4daf3c080e0c31911befc16f0cc4a09908bb7c1e26abab38bd7b788e1a09c0edf1a35a38d2ff1d3ed47fcdaae2f0934224694f5b56705b9409b6d3d64f3833b686f7576ec64bbdd6ff174e56c2d1edac0011f904681a73face26573fbba4e34652f7ae84acfb2fa5a5b3046f98178cd0831df7477de70e06a4c00e305f31aafc026ef064dd68fd3e4252b1b91d617b26c6d09b6891a00df68f105b5962e7f9d82da101dd595d286da721443b72b2aba2377f6e7772e33b3a5e3753da9c2578c5d1daab80187f55518c72a64ee150a7cb5649823c08c9f62cd7d020b45ec2cba8310db1a7785a46ab24785b4d54ff1660b5ca78e05a9a55edba9c60bf044737bc468101c4e8bd1480d749be5024adefca1d998abe33eaeb6b11fbb39da5d905fdd3f611b2e51517ccee4b8af72c2d948573505590d61a6783ab7278fc43fe55b1fcc0e7216444d3c8039bb8145ef1ce01c50e95a3f3feab0aee883fdb94cc13ee4d21c542aa795e18932228981690f4d4c57ca4db6eb5c092e29d8a05139d509a8aeb48baa1eb97a76e597a32b280b5e9d6c36859064c98ff96ef5126130264fa8d2f49213870d9fb036cff95da51f270311d9976208554e48ffd486470d0ecdb4e619ccbd8226147204baf8e235f54d8b1cba8fa34a9a4d055de515cdf180d2bb6739a175183c472e30b5c914d09eeb1b7dafd6872b38b48c6afc146101200e6e6a44fe5684e220adc11f5c403ddb15df8051e6bdef09117a3a5349938513776286473a3cf1d2788bb875052a2e6459fa7926da33380149c7f98d7700528a60c954e6f5ecb65842fde69d614be69eaa2040a4819ae6e756accf936e14c1e894489744a79c1f2c1eb295d13e2d767c09964b61f9cfe497649f712",
        false,
        "cdeadcb34e0f77beab60668405970965",
        "f582a614ef9d92bcb500405798bc9fe1",
        "044739f35128e1bdcc119ddbcf1646d5cf32df2a3fb644c24e588a390d6a3c95",
        "8f0ca62610b20b785b87839e8e185c62467e4ff560f1a750dc0e4ace157fbe025b6b8cf3087a02228e74851ab3d35e99144ab472ba5e524aa0b40b2371dfdef9b3a321f5da99126c45a538e5cfb2d3d7084f70",
        "");
    TestBIP324PacketVector(
        223,
        "6c77432d1fda31e9f942f8af44607e10f3ad38a65f8a4bddae823e5eff90dc38",
        "d2685070c1e6376e633e825296634fd461fa9e5bdf2109bcebd735e5a91f3e587c5cb782abb797fbf6bb5074fd1542a474f2a45b673763ec2db7fb99b737bbb9",
        "56bd0c06f10352c3a1a9f4b4c92f6fa2b26df124b57878353c1fc691c51abea77c8817daeeb9fa546b77c8daf79d89b22b0e1b87574ece42371f00237aa9d83a",
        false,
        "7e0e78eb6990b059e6cf0ded66ea93ef82e72aa2f18ac24f2fc6ebab561ae557420729da103f64cecfa20527e15f9fb669a49bbbf274ef0389b3e43c8c44e5f60bf2ac38e2b55e7ec4273dba15ba41d21f8f5b3ee1688b3c29951218caf847a97fb50d75a86515d445699497d968164bf740012679b8962de573be941c62b7ef",
        1,
        "",
        true,
        "fd75e4ef7173e168703b5fecab4e2b74",
        "6b76348ded74a39f8416d693be3acf83",
        "5d57127e10962b15e9f7ccfa3e817dbba1559fe3e3256779febb57fdceb720e8",
        "",
        "379d2b1bf4a29494ac0e14f2468dd351e31855ee0e0e12e233347c3e65b29fd03d94a20289887e570ab4b5254ded5ef3df9a2273dc7cfae5b3282a76c09db68a1f801198a72a8cd409a64b2a660c243a11459272d5b6b276582d707449e10e2a8783be2bb84b89bffbc18bc41a5a634219bdafd3041051ccc4308a2894f82ef5");
    TestBIP324PacketVector(
        448,
        "a6ec25127ca1aa4cf16b20084ba1e6516baae4d32422288e9b36d8bddd2de35a",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff053d7ecca53e33e185a8b9be4e7699a97c6ff4c795522e5918ab7cd6b6884f67e683f3dc",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffa7730be30000000000000000000000000000000000000000000000000000000000000000",
        true,
        "00cf68f8f7ac49ffaa02c4864fdf6dfe7bbf2c740b88d98c50ebafe32c92f3427f57601ffcb21a3435979287db8fee6c302926741f9d5e464c647eeb9b7acaeda46e00abd7506fc9a719847e9a7328215801e96198dac141a15c7c2f68e0690dd1176292a0dded04d1f548aad88f1aebdc0a8f87da4bb22df32dd7c160c225b843e83f6525d6d484f502f16d923124fc538794e21da2eb689d18d87406ecced5b9f92137239ed1d37bcfa7836641a83cf5e0a1cf63f51b06f158e499a459ede41c",
        1,
        "",
        false,
        "cee8366e8acff83a5e1269753b0abcfb",
        "92a1cb1111ec2896f1e97d7a138b6eb8",
        "54b7b78b6f916b5638109a7c48255e2cc63346e30e02e3bd0d608577adaea31c",
        "",
        "f807659d1c69143dd864444656d88badc1c933e764ba8d4243dae88dca419f7c5a498884d458cf5499c0710153cad49374fca6cc342e5c9dd897aa46805866b75bd2e9d011c95a3e5b826347e30fbaf2e4725ed0a3d2818fe086d84df6ae99bd9bd41cdcebce80e19e9508c8ab1c9334dfead6ce09d2c02e4d5c2a3ef284bfa8");
    TestBIP324PacketVector(
        673,
        "0af952659ed76f80f585966b95ab6e6fd68654672827878684c8b547b1b94f5a",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffc81017fd92fd31637c26c906b42092e11cc0d3afae8d9019d2578af22735ce7bc469c72d",
        "9652d78baefc028cd37a6a92625b8b8f85fde1e4c944ad3f20e198bef8c02f19fffffffffffffffffffffffffffffffffffffffffffffffffffffffff2e91870",
        false,
        "5c6272ee55da855bbbf7b1246d9885aa7aa601a715ab86fa46c50da533badf82b97597c968293ae04e",
        97561,
        "",
        false,
        "a2540f12d510409cbe564d176519f2f3",
        "a8a9211a0874dd88fad7de637c05667a",
        "1aeeafadeeb1eb22de7544b97cb00b2820cc2869bd7d41dbf6abe6f48506a26f",
        "",
        "d14b486664fb324a83c1407ecb487fa5d03bcf225e35c89f56948d3120711070290d2a7883749359e7f8aeca641b0a7ae211ff9592cfb2dc5f838e5639bedbd23e7887265d026d775f6729575d87db3ecdb96770ca43aa21d8418466e5942dd9cd447fe146c6c5eefb0caf793d9e21d7eb85f9ec13638d25d5e45ddcb8fe1497");
    TestBIP324PacketVector(
        1024,
        "f90e080c64b05824c5a24b2501d5aeaf08af3872ee860aa80bdcd430f7b63494",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff115173765dc202cf029ad3f15479735d57697af12b0131dd21430d5772e4ef11474d58b9",
        "12a50f3fafea7c1eeada4cf8d33777704b77361453afc83bda91eef349ae044d20126c6200547ea5a6911776c05dee2a7f1a9ba7dfbabbbd273c3ef29ef46e46",
        true,
        "5f67d15d22ca9b2804eeab0a66f7f8e3a10fa5de5809a046084348cbc5304e843ef96f59a59c7d7fdfe5946489f3ea297d941bac326225df316a25fc90f0e65b0d31a9c497e960fdbf8c482516bc8a9c1c77b7f6d0e1143810c737f76f9224e6f2c9af5186b4f7259c7e8d165b6e4fe3d38a60bdbdd4d06ecdcaaf62086070dbb68686b802d53dfd7db14b18743832605f5461ad81e2af4b7e8ff0eff0867a25b93cec7becf15c43131895fed09a83bf1ee4a87d44dd0f02a837bf5a1232e201cb882734eb9643dc2dc4d4e8b5690840766212c7ac8f38ad8a9ec47c7a9b3e022ae3eb6a32522128b518bd0d0085dd81c5",
        69615,
        "",
        true,
        "966be2089d4d71c1685e1c0b7fb1d4a8",
        "2ebbbed09918e6b4f40fa11e2afffba7",
        "28fb11d5feb38102db5424eacfc94caac2d43455bc512976f25c2df51e6ccedc",
        "",
        "7b3e272da2ae9091e33a12eaffa8141737e59c0eab5679ffddceafc5964d5d9910dcbfc614ed6eb5b6a75db1bf2d8f6c022410f21da3e13de2808c391ae032b8a1b07a5677ac044be936eec14f8a12b4fb96e8650490a5a6020b310c5f877e889c95276eb40acbe1e17cd8d6d09c63a555cb0f22311d63af4dd099fe3b7e322c");
}

BOOST_AUTO_TEST_SUITE_END()