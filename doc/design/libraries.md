# Libraries

| Name                     | Description |
|--------------------------|-------------|
| *libgriffion_cli*         | RPC client functionality used by *griffion-cli* executable |
| *libgriffion_common*      | Home for common functionality shared by different executables and libraries. Similar to *libgriffion_util*, but higher-level (see [Dependencies](#dependencies)). |
| *libgriffion_consensus*   | Stable, backwards-compatible consensus functionality used by *libgriffion_node* and *libgriffion_wallet* and also exposed as a [shared library](../shared-libraries.md). |
| *libgriffion_kernel*      | Consensus engine and support library used for validation by *libgriffion_node* and also exposed as a [shared library](../shared-libraries.md). |
| *libgriffionqt*           | GUI functionality used by *griffion-qt* and *griffion-gui* executables |
| *libgriffion_ipc*         | IPC functionality used by *griffion-node*, *griffion-wallet*, *griffion-gui* executables to communicate when [`--enable-multiprocess`](multiprocess.md) is used. |
| *libgriffion_node*        | P2P and RPC server functionality used by *griffiond* and *griffion-qt* executables. |
| *libgriffion_util*        | Home for common functionality shared by different executables and libraries. Similar to *libgriffion_common*, but lower-level (see [Dependencies](#dependencies)). |
| *libgriffion_wallet*      | Wallet functionality used by *griffiond* and *griffion-wallet* executables. |
| *libgriffion_wallet_tool* | Lower-level wallet functionality used by *griffion-wallet* executable. |
| *libgriffion_zmq*         | [ZeroMQ](../zmq.md) functionality used by *griffiond* and *griffion-qt* executables. |

## Conventions

- Most libraries are internal libraries and have APIs which are completely unstable! There are few or no restrictions on backwards compatibility or rules about external dependencies. Exceptions are *libgriffion_consensus* and *libgriffion_kernel* which have external interfaces documented at [../shared-libraries.md](../shared-libraries.md).

- Generally each library should have a corresponding source directory and namespace. Source code organization is a work in progress, so it is true that some namespaces are applied inconsistently, and if you look at [`libgriffion_*_SOURCES`](../../src/Makefile.am) lists you can see that many libraries pull in files from outside their source directory. But when working with libraries, it is good to follow a consistent pattern like:

  - *libgriffion_node* code lives in `src/node/` in the `node::` namespace
  - *libgriffion_wallet* code lives in `src/wallet/` in the `wallet::` namespace
  - *libgriffion_ipc* code lives in `src/ipc/` in the `ipc::` namespace
  - *libgriffion_util* code lives in `src/util/` in the `util::` namespace
  - *libgriffion_consensus* code lives in `src/consensus/` in the `Consensus::` namespace

## Dependencies

- Libraries should minimize what other libraries they depend on, and only reference symbols following the arrows shown in the dependency graph below:

<table><tr><td>

```mermaid

%%{ init : { "flowchart" : { "curve" : "basis" }}}%%

graph TD;

griffion-cli[griffion-cli]-->libgriffion_cli;

griffiond[griffiond]-->libgriffion_node;
griffiond[griffiond]-->libgriffion_wallet;

griffion-qt[griffion-qt]-->libgriffion_node;
griffion-qt[griffion-qt]-->libgriffionqt;
griffion-qt[griffion-qt]-->libgriffion_wallet;

griffion-wallet[griffion-wallet]-->libgriffion_wallet;
griffion-wallet[griffion-wallet]-->libgriffion_wallet_tool;

libgriffion_cli-->libgriffion_util;
libgriffion_cli-->libgriffion_common;

libgriffion_common-->libgriffion_consensus;
libgriffion_common-->libgriffion_util;

libgriffion_kernel-->libgriffion_consensus;
libgriffion_kernel-->libgriffion_util;

libgriffion_node-->libgriffion_consensus;
libgriffion_node-->libgriffion_kernel;
libgriffion_node-->libgriffion_common;
libgriffion_node-->libgriffion_util;

libgriffionqt-->libgriffion_common;
libgriffionqt-->libgriffion_util;

libgriffion_wallet-->libgriffion_common;
libgriffion_wallet-->libgriffion_util;

libgriffion_wallet_tool-->libgriffion_wallet;
libgriffion_wallet_tool-->libgriffion_util;

classDef bold stroke-width:2px, font-weight:bold, font-size: smaller;
class griffion-qt,griffiond,griffion-cli,griffion-wallet bold
```
</td></tr><tr><td>

**Dependency graph**. Arrows show linker symbol dependencies. *Consensus* lib depends on nothing. *Util* lib is depended on by everything. *Kernel* lib depends only on consensus and util.

</td></tr></table>

- The graph shows what _linker symbols_ (functions and variables) from each library other libraries can call and reference directly, but it is not a call graph. For example, there is no arrow connecting *libgriffion_wallet* and *libgriffion_node* libraries, because these libraries are intended to be modular and not depend on each other's internal implementation details. But wallet code is still able to call node code indirectly through the `interfaces::Chain` abstract class in [`interfaces/chain.h`](../../src/interfaces/chain.h) and node code calls wallet code through the `interfaces::ChainClient` and `interfaces::Chain::Notifications` abstract classes in the same file. In general, defining abstract classes in [`src/interfaces/`](../../src/interfaces/) can be a convenient way of avoiding unwanted direct dependencies or circular dependencies between libraries.

- *libgriffion_consensus* should be a standalone dependency that any library can depend on, and it should not depend on any other libraries itself.

- *libgriffion_util* should also be a standalone dependency that any library can depend on, and it should not depend on other internal libraries.

- *libgriffion_common* should serve a similar function as *libgriffion_util* and be a place for miscellaneous code used by various daemon, GUI, and CLI applications and libraries to live. It should not depend on anything other than *libgriffion_util* and *libgriffion_consensus*. The boundary between _util_ and _common_ is a little fuzzy but historically _util_ has been used for more generic, lower-level things like parsing hex, and _common_ has been used for griffion-specific, higher-level things like parsing base58. The difference between util and common is mostly important because *libgriffion_kernel* is not supposed to depend on *libgriffion_common*, only *libgriffion_util*. In general, if it is ever unclear whether it is better to add code to *util* or *common*, it is probably better to add it to *common* unless it is very generically useful or useful particularly to include in the kernel.


- *libgriffion_kernel* should only depend on *libgriffion_util* and *libgriffion_consensus*.

- The only thing that should depend on *libgriffion_kernel* internally should be *libgriffion_node*. GUI and wallet libraries *libgriffionqt* and *libgriffion_wallet* in particular should not depend on *libgriffion_kernel* and the unneeded functionality it would pull in, like block validation. To the extent that GUI and wallet code need scripting and signing functionality, they should be get able it from *libgriffion_consensus*, *libgriffion_common*, and *libgriffion_util*, instead of *libgriffion_kernel*.

- GUI, node, and wallet code internal implementations should all be independent of each other, and the *libgriffionqt*, *libgriffion_node*, *libgriffion_wallet* libraries should never reference each other's symbols. They should only call each other through [`src/interfaces/`](`../../src/interfaces/`) abstract interfaces.

## Work in progress

- Validation code is moving from *libgriffion_node* to *libgriffion_kernel* as part of [The libgriffionkernel Project #24303](https://github.com/GriffionProject/Griffion/issues/24303)
- Source code organization is discussed in general in [Library source code organization #15732](https://github.com/GriffionProject/Griffion/issues/15732)
