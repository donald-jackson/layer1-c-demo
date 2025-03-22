# Demo from terminal output

## Set vars

```
layer1-demo: export CLIENT_ID="c945e4d2-1a5c-49cd-aa8b-d4a9583b28b5"
layer1-demo: export ASSET_POOL_ID="31455265-2aab-431a-9d0d-0818dae0534d"
```

## Create deposit addresses for USDT

```
./layer1_cli --client-id $CLIENT_ID --key-file ../private_key.pem create-address-by-asset --asset-pool-id $ASSET_POOL_ID --asset USDT --reference donald-demo-1
Address creation initiated...
Addresses created:
Network: TRON       Address: TK9wH8uiYeG8ugSpaiemFBQRB1dYq5tUGM
Network: SOLANA     Address: 7Rj39Uu1QYGWsKRBoniG7o5fzhtdbVC4vhrwBRBg3MFm
Network: POLYGON    Address: 0x64c095398ec63ef39e162f557929e1cf426bc137
Network: BINANCE    Address: 0x64c095398ec63ef39e162f557929e1cf426bc137
Network: ETHEREUM   Address: 0x64c095398ec63ef39e162f557929e1cf426bc137
```

## List transactions by reference (user)

```
./layer1_cli --client-id $CLIENT_ID --key-file ../private_key.pem list-transactions --asset-pool-id $ASSET_POOL_ID --reference donald-withdrawal-demo-1                        
Transactions found:

Transaction 1:
  ID: 0195bb814a567916aae8109f276eb8fd3524d51c2ab64031ba3613afc85b747869455448
  Status: SUCCESS
  Network: ETHEREUM
  Asset: ETH
  Reference: REF-202277088R5N6OWJCHJ
  Created At: 2025-03-22T01:39:27.446Z
  Amount: 0.020126058226553
```


## Send a transaction to user, 1st call

```
./layer1_cli --client-id $CLIENT_ID --key-file ../private_key.pem create-transaction --asset-pool-id $ASSET_POOL_ID --asset ETH --network ETHEREUM --amount 0.02 --to 0x046F4227EF6E397Db6b5607F86A0AE5f6C54805b --reference donald-withdrawal-demo-1
Transaction created successfully:
  ID: 0195bb81-4a56-7916-aae8-109f276eb8fd
  Status: CREATED
  Network: ETHEREUM
  Asset: ETH
  Reference: donald-withdrawal-demo-1
  Created At: 2025-03-22T01:39:27.446Z
```

## Send a transaction to user, 2nd call, same reference

* Was already processed so just returns current state

```
./layer1_cli --client-id $CLIENT_ID --key-file ../private_key.pem create-transaction --asset-pool-id $ASSET_POOL_ID --asset ETH --network ETHEREUM --amount 0.02 --to 0x046F4227EF6E397Db6b5607F86A0AE5f6C54805b --reference donald-withdrawal-demo-1
Transaction created successfully:
  ID: 0195bb81-4a56-7916-aae8-109f276eb8fd
  Status: SUCCESS
  Network: ETHEREUM
  Asset: ETH
  Reference: donald-withdrawal-demo-1
  Created At: 2025-03-22T01:39:27.446Z
```