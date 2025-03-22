# Layer1 CLI

A command-line interface for interacting with the Layer1 Digital API.

## Dependencies

### Mac OS (Homebrew)

```bash
# Install dependencies
brew install cmake openssl curl

# If OpenSSL is installed with Homebrew, you might need to set these environment variables
export OPENSSL_ROOT_DIR=$(brew --prefix openssl)
export OPENSSL_INCLUDE_DIR=$OPENSSL_ROOT_DIR/include
```

### Debian/Ubuntu Linux (apt)

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev libcurl4-openssl-dev
```

## Building

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make
```

## Usage

```bash
# Basic usage
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> <command> [args...]

# Example: Create a new address
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> create-address --asset-pool-id <pool-id> --network <network> --asset <asset> --reference <ref>

# Example: Create addresses for all networks by asset
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> create-address-by-asset --asset-pool-id <pool-id> --asset <asset> --reference <ref>
```

### Commands

#### create-address

Creates a new address for receiving assets.

```bash
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> create-address --asset-pool-id <pool-id> --network <network> --asset <asset> --reference <ref>
```

Arguments:
- `asset-pool-id`: The ID of the asset pool
- `network`: The network (e.g., ETHEREUM, TRON, SOLANA, POLYGON, BINANCE)
- `asset`: The asset (e.g., USDT, USDC, ETH)
- `reference`: A reference for the address

#### create-address-by-asset

Creates addresses for all supported networks for a specific asset.

```bash
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> create-address-by-asset --asset-pool-id <pool-id> --asset <asset> --reference <ref>
```

Arguments:
- `asset-pool-id`: The ID of the asset pool
- `asset`: The asset (e.g., USDT, USDC, ETH)
- `reference`: A reference for the addresses

This command will create addresses for all supported networks for the specified asset and poll for updates until all addresses are created.

#### create-transaction

Creates a new blockchain transaction.

```bash
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> create-transaction --asset-pool-id <pool-id> --network <network> --asset <asset> --to <address> --amount <amount> [--reference <ref>]
```

Arguments:
- `asset-pool-id`: The ID of the asset pool
- `network`: The network (e.g., ETHEREUM, TRON, SOLANA, POLYGON, BINANCE)
- `asset`: The asset (e.g., USDT, USDC, ETH)
- `to`: The destination address
- `amount`: The amount to transfer
- `reference` (optional): A reference for the transaction

#### list-transactions

Lists transactions by reference.

```bash
./layer1_cli --client-id <client-id> --key-file <path-to-private-key> list-transactions --asset-pool-id <pool-id> --reference <ref>
```

Arguments:
- `asset-pool-id`: The ID of the asset pool
- `reference`: The reference to search for

The command will display all transactions (deposits and withdrawals) associated with the given reference.

## Development

### Adding New Commands

To add a new command:

1. Create a new header file in `include/commands/`
2. Create a new implementation file in `src/commands/`
3. Register the command in `init_commands()` in `main.c`