// Copyright (c) 2011-2020 The Bitcoin Core developers
// Copyright (c) 2024 The Griffion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GRIFFION_QT_GRIFFIONADDRESSVALIDATOR_H
#define GRIFFION_QT_GRIFFIONADDRESSVALIDATOR_H

#include <QValidator>

/** Base58 entry widget validator, checks for valid characters and
 * removes some whitespace.
 */
class GriffionAddressEntryValidator : public QValidator
{
    Q_OBJECT

public:
    explicit GriffionAddressEntryValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

/** Griffion address widget validator, checks for a valid griffion address.
 */
class GriffionAddressCheckValidator : public QValidator
{
    Q_OBJECT

public:
    explicit GriffionAddressCheckValidator(QObject *parent);

    State validate(QString &input, int &pos) const override;
};

#endif // GRIFFION_QT_GRIFFIONADDRESSVALIDATOR_H
