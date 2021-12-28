# cee-bot

Bot for the [Cee.Studio](https://discord.gg/nBUqrWf) server.

## Getting Started

1. Get [Orca](https://github.com/cee-studio/orca) by following its installation steps
2. Edit [config.json](config.json) and match its fields to your guild and bot primitives
3. Head to [listeners/](listeners/) and follow its guide to activate the interaction listeners
4. Build the bot
  ```bash
  $ make
  ```
5. Finally, run the bot
  ```bash
  $ ./main
  ```

## Features

### Rubber duck channel

A special channel that users may create to have their own space where they may
ask for specialized help in their projects, or simply throw their developer thoughts to.

#### `/mychannel action` - Moderation commands for your channel
#### `/mychannel configure` - Configure your channel
#### `/mychannel delete` - Delete your channel

## Project outline

```
.
├── config.json    # The client primitives
├── interactions/  # Logic for client reaction to interactions
└── listeners/     # Activate / Update Interaction listeners
```
