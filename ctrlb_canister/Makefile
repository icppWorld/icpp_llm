SHELL := /bin/bash

# Disable built-in rules and variables
MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables

NETWORK ?= local
DFX_WEBSERVER_PORT ?= $(shell dfx info webserver-port)
IDENTITY ?= $(shell dfx identity whoami)

###########################################################################
# Some constants
CANISTER_INSTALL_MODE ?= install
CANISTER_CANDID_UI_IC ?= "a4gq6-oaaaa-aaaab-qaa4q-cai"

# Some defaults
CANISTER_NAME ?= ctrlb_canister


.PHONY: dfx-cycles-to-canister
dfx-cycles-to-canister:
	@$(eval CANISTER_ID := $(shell dfx canister --network ic id ctrlb_canister))
	@echo "-------------------------------------------------------------------------"
	@echo "dfx identity             : $(IDENTITY)"
	@echo "- balance before: "
	@dfx wallet --network ic balance
	@echo "-------------------------------------------------------------------------"
	@echo "ctrlb_canister  before    : $(CANISTER_ID)"
	@dfx canister --network=ic status ctrlb_canister
	@echo "-------------------------------------------------------------------------"
	@echo "Sending 0.5T cycles to ctrlb_canister"
	dfx wallet --network ic send $(CANISTER_ID) 500000000000
	@echo "-------------------------------------------------------------------------"
	@echo "dfx identity             : $(IDENTITY)"
	@echo "- balance after: "
	@dfx wallet --network ic balance
	@echo "-------------------------------------------------------------------------"
	@echo "ctrlb_canister  after    : $(CANISTER_ID)"
	@dfx canister --network=ic status ctrlb_canister

.PHONY: dfx-canisters-of-project-ic
dfx-canisters-of-project-ic:
	@$(eval IDENTITY_PRINCIPAL := $(shell dfx identity --network ic get-principal))
	@$(eval IDENTITY_CYCLES_WALLET := $(shell dfx identity --network ic get-wallet))
	@$(eval IDENTITY_ICP_WALLET := $(shell dfx ledger --network ic account-id))
	@$(eval IDENTITY_ICP_BALANCE := $(shell dfx ledger --network ic balance))
	@$(eval crtlb_canister_ID := $(shell dfx canister --network ic id ctrlb_canister))

	@echo '-------------------------------------------------'
	@echo "NETWORK                  : ic"
	@echo "dfx identity             : $(IDENTITY)"
	@echo "identity's principal     : $(IDENTITY_PRINCIPAL)"
	@echo "identity's cycles wallet : $(IDENTITY_CYCLES_WALLET)"
	@echo "identity's ICP wallet    : $(IDENTITY_ICP_WALLET)"
	@echo "identity's ICP balance   : $(IDENTITY_ICP_BALANCE)"
	@echo '-------------------------------------------------'
	@echo "identity's cycles wallet : $(IDENTITY_CYCLES_WALLET)"
	@echo "- balance: "
	@dfx wallet --network ic balance
	@echo "- status: "
	@dfx canister --network=ic status $(IDENTITY_CYCLES_WALLET)
	@echo '-------------------------------------------------'
	@echo "ctrlb_canister         : $(crtlb_canister_ID)"
	@dfx canister --network=ic status ctrlb_canister
	@echo '-------------------------------------------------'
	@echo 'View in browser at:'
	@echo  "identity's wallet              : https://$(IDENTITY_CYCLES_WALLET).raw.ic0.app/"
	@echo  "Candid UI                      : https://$(CANISTER_CANDID_UI_IC).raw.ic0.app/"
	

.PHONY: dfx-wallet-details
dfx-wallet-details:
	@$(eval IDENTITY_CYCLES_WALLET := $(shell dfx identity --network $(NETWORK) get-wallet))
	@echo "-------------------------------------------------------------------------"
	@echo "make dfx-wallet-details NETWORK=$(NETWORK)"
	@if [[ ${NETWORK} == "ic" ]]; then \
		echo  "View details at         : https://$(IDENTITY_CYCLES_WALLET).raw.ic0.app/"; \
	else \
		echo  "View details at         : ?? http://localhost:$(DFX_WEBSERVER_PORT)?canisterId=$(IDENTITY_CYCLES_WALLET) ?? "; \
	fi
	
	@echo "-------------------------------------------------------------------------"
	@echo -n "cycles canister id      : " && dfx identity --network $(NETWORK) get-wallet
	@echo -n "cycles canister name    : " && dfx wallet --network $(NETWORK) name
	@echo -n "cycles canister balance : " && dfx wallet --network $(NETWORK) balance
	@echo "-------------------------------------------------------------------------"
	@echo "controllers: "
	@dfx wallet --network $(NETWORK) controllers
	@echo "-------------------------------------------------------------------------"
	@echo "custodians: "
	@dfx wallet --network $(NETWORK) custodians
	@echo "-------------------------------------------------------------------------"
	@echo "addresses: "
	@dfx wallet --network $(NETWORK) addresses

.PHONY: dfx-wallet-controller-add
dfx-wallet-controller-add:
	@[ "${PRINCIPAL}" ]	|| ( echo ">> Define PRINCIPAL to add as controller: 'make dfx-cycles-controller-add PRINCIPAL=....' "; exit 1 )
	@echo    "NETWORK         : $(NETWORK)"
	@echo    "PRINCIPAL       : $(PRINCIPAL)"
	@dfx wallet --network $(NETWORK) add-controller $(PRINCIPAL)

.PHONY: dfx-wallet-controller-remove
dfx-wallet-controller-remove:
	@[ "${PRINCIPAL}" ]	|| ( echo ">> Define PRINCIPAL to remove as controller: 'make dfx-cycles-controller-remove PRINCIPAL=....' "; exit 1 )
	@echo    "NETWORK         : $(NETWORK)"
	@echo    "PRINCIPAL       : $(PRINCIPAL)"
	@dfx wallet --network $(NETWORK) remove-controller $(PRINCIPAL)