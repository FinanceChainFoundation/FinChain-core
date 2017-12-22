FinChain Core
==============

FinChain Core is the FinChain blockchain implementation and command-line interface.
The web wallet is [FinChain UI](https://github.com/FinanceChainFoundation/FinChain-UI).

repositories can be updated with the following steps:

    git remote set-url origin https://github.com/FinanceChainFoundation/FinChain-core.git
    git checkout master
    git remote set-head origin --auto
    git pull
    git submodule sync --recursive
    git submodule update --init --recursive

Getting Started
---------------

We recommend building on Ubuntu 16.04 LTS, and the build dependencies may be installed with:

    sudo apt-get update
    sudo apt-get install autoconf cmake git libboost-all-dev libssl-dev

To build after all dependencies are installed:

    git clone https://github.com/FinanceChainFoundation/FinChain-core.git
    cd FinChain-core
    git checkout <LATEST_RELEASE_TAG>
    git submodule update --init --recursive
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
 to build witness_node use:
    make witness_node 
 to build cli_wallet use:
    make cli_wallet

**NOTE:** FinChain requires an [OpenSSL](https://www.openssl.org/) version in the 1.0.x series. OpenSSL 1.1.0 and newer are NOT supported. If your system OpenSSL version is newer, then you will need to manually provide an older version of OpenSSL and specify it to CMake using `-DOPENSSL_INCLUDE_DIR`, `-DOPENSSL_SSL_LIBRARY`, and `-DOPENSSL_CRYPTO_LIBRARY`.

**NOTE:** FinChain requires a [Boost](http://www.boost.org/) version in the range [1.57, 1.60]. Versions earlier than
1.57 or newer than 1.60 are NOT supported. If your system Boost version is newer, then you will need to manually build
an older version of Boost and specify it to CMake using `DBOOST_ROOT`.

After building, the witness node can be launched with:

    ./programs/witness_node/witness_node

The node will automatically create a data directory including a config file. It may take several hours to fully synchronize
the blockchain. After syncing, you can exit the node using Ctrl+C and setup the command-line wallet by editing
`witness_node_data_dir/config.ini` as follows:

    rpc-endpoint = 127.0.0.1:8090

After starting the witness node again, in a separate terminal you can run:

    ./programs/cli_wallet/cli_wallet

Set your inital password:

    >>> set_password <PASSWORD>
    >>> unlock <PASSWORD>

To import your initial balance:

    >>> import_balance <ACCOUNT NAME> [<WIF_KEY>] true

If you send private keys over this connection, `rpc-endpoint` should be bound to localhost for security.

Use `help` to see all available wallet commands. Source definition and listing of all commands is available
[here](https://github.com/FinanceChainFoundation/FinChain-core/blob/master/libraries/wallet/include/graphene/wallet/wallet.hpp).

Support
-------

FinChain Core bugs can be reported directly to the [issue tracker](https://github.com/FinanceChainFoundation/FinChain-core/issues).

FinChain UI bugs should be reported to the [UI issue tracker](https://github.com/FinanceChainFoundation/FinChain-UI/issues)

Using the API
-------------

We provide several different API's.  Each API has its own ID.
When running `witness_node`, initially two API's are available:
API 0 provides read-only access to the database, while API 1 is
used to login and gain access to additional, restricted API's.

Here is an example using `wscat` package from `npm` for websockets:

    $ npm install -g wscat
    $ wscat -c ws://127.0.0.1:8090
    > {"id":1, "method":"call", "params":[0,"get_accounts",[["1.2.0"]]]}
    < {"id":1,"result":[{"id":"1.2.0","annotations":[],"membership_expiration_date":"1969-12-31T23:59:59","registrar":"1.2.0","referrer":"1.2.0","lifetime_referrer":"1.2.0","network_fee_percentage":2000,"lifetime_referrer_fee_percentage":8000,"referrer_rewards_percentage":0,"name":"committee-account","owner":{"weight_threshold":1,"account_auths":[],"key_auths":[],"address_auths":[]},"active":{"weight_threshold":6,"account_auths":[["1.2.5",1],["1.2.6",1],["1.2.7",1],["1.2.8",1],["1.2.9",1],["1.2.10",1],["1.2.11",1],["1.2.12",1],["1.2.13",1],["1.2.14",1]],"key_auths":[],"address_auths":[]},"options":{"memo_key":"GPH1111111111111111111111111111111114T1Anm","voting_account":"1.2.0","num_witness":0,"num_committee":0,"votes":[],"extensions":[]},"statistics":"2.7.0","whitelisting_accounts":[],"blacklisting_accounts":[]}]}

We can do the same thing using an HTTP client such as `curl` for API's which do not require login or other session state:

    $ curl --data '{"jsonrpc": "2.0", "method": "call", "params": [0, "get_accounts", [["1.2.0"]]], "id": 1}' http://127.0.0.1:8090/rpc
    {"id":1,"result":[{"id":"1.2.0","annotations":[],"membership_expiration_date":"1969-12-31T23:59:59","registrar":"1.2.0","referrer":"1.2.0","lifetime_referrer":"1.2.0","network_fee_percentage":2000,"lifetime_referrer_fee_percentage":8000,"referrer_rewards_percentage":0,"name":"committee-account","owner":{"weight_threshold":1,"account_auths":[],"key_auths":[],"address_auths":[]},"active":{"weight_threshold":6,"account_auths":[["1.2.5",1],["1.2.6",1],["1.2.7",1],["1.2.8",1],["1.2.9",1],["1.2.10",1],["1.2.11",1],["1.2.12",1],["1.2.13",1],["1.2.14",1]],"key_auths":[],"address_auths":[]},"options":{"memo_key":"GPH1111111111111111111111111111111114T1Anm","voting_account":"1.2.0","num_witness":0,"num_committee":0,"votes":[],"extensions":[]},"statistics":"2.7.0","whitelisting_accounts":[],"blacklisting_accounts":[]}]}

API 0 is accessible using regular JSON-RPC:

    $ curl --data '{"jsonrpc": "2.0", "method": "get_accounts", "params": [["1.2.0"]], "id": 1}' http://127.0.0.1:8090/rpc

Accessing restricted API's
--------------------------

You can restrict API's to particular users by specifying an `apiaccess` file in `config.ini`.  Here is an example `apiaccess` file which allows
user `bytemaster` with password `supersecret` to access four different API's, while allowing any other user to access the three public API's
necessary to use the wallet:

    {
       "permission_map" :
       [
          [
             "bytemaster",
             {
                "password_hash_b64" : "9e9GF7ooXVb9k4BoSfNIPTelXeGOZ5DrgOYMj94elaY=",
                "password_salt_b64" : "INDdM6iCi/8=",
                "allowed_apis" : ["database_api", "network_broadcast_api", "history_api", "network_node_api"]
             }
          ],
          [
             "*",
             {
                "password_hash_b64" : "*",
                "password_salt_b64" : "*",
                "allowed_apis" : ["database_api", "network_broadcast_api", "history_api"]
             }
          ]
       ]
    }

Passwords are stored in `base64` as salted `sha256` hashes.  A simple Python script, `saltpass.py` is avaliable to obtain hash and salt values from a password.
A single asterisk `"*"` may be specified as username or password hash to accept any value.

With the above configuration, here is an example of how to call `add_node` from the `network_node` API:

    {"id":1, "method":"call", "params":[1,"login",["bytemaster", "supersecret"]]}
    {"id":2, "method":"call", "params":[1,"network_node",[]]}
    {"id":3, "method":"call", "params":[2,"add_node",["127.0.0.1:9090"]]}

Note, the call to `network_node` is necessary to obtain the correct API identifier for the network API.  It is not guaranteed that the network API identifier will always be `2`.

License
-------
FinChain Core is under the MIT license. See [LICENSE](https://github.com/FinanceChainFoundation/FinChain-core/blob/master/LICENSE.txt)
for more information.
