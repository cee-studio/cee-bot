# cee-bot

Bot for the [Cee.Studio](https://discord.gg/nBUqrWf) server.

## Getting Started

1. Open [config.json](config.json) and match the `cee_bot` field to your server primitives
2. Head to [listeners/](listeners/) and follow its guide to activate the interaction listeners
3. Build the bot
  ```bash
  $ make
  ```
4. And finally, run the bot
  ```bash
  $ ./main
  ```

## Project outline

```
.
├── config.json    # The client primitives
├── interactions/  # Logic for client reaction to interactions
└── listeners/     # Activate / Update Interaction listeners
```
