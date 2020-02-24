all: lint

lint: dtslint cpplint
dtslint:
	npx dtslint types
cpplint:
	cpplint --filter=-,+build,+runtime,-runtime/indentation_namespace binding/*.cpp binding/*.hpp
.PHONY: lint dtslint cpplint

docs:
	cd types && npx typedoc
.PHONY: docs
