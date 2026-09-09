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

1. the bot only registers slash commands in servers its already in, it does not register them when it joins a new server, the bot must be restarted

2. pairs is also only updated based on whats in the database on server start, not after, the bot must be restarted upon channel registration

3. images and gifs dont embed due to the embed containing user info being there already, currently thinking about wether i should remove embed in favour of pure text which would allow for gifs and images to embed



