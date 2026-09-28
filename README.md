# Cross communicator

a discord bot that allows communication between multiple servers, written in C with the concord library, good for entire communities

## build steps

run

```
make prod
```
in the root directory of the project

there will be config.json and and data.db, data.db doesnt need to be touched but config.json must be edited to have your discord token, so that the bot can actually function
the bot must also have message content intent enabled, not doing so prevents message content from being cross communicated

## current issues to resolve

1. pairs is also only updated based on whats in the database on server start, not after, the bot must be restarted upon channel registration




