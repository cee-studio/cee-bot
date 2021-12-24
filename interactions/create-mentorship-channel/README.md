## Interaction - `Create Mentorship Channel`

### `set.c` - Send the interaction message to guild's `#rules` channel

1. Modify `components.json` to change how the interaction message is structured
2. Modify the `cee_bot` field from `config.json` with your server primitives
3. Build with 
   ```bash
   $ make
   ```
4. Run `./set`

### `react.c` - React to the interaction

This file consists of the react logic for this interaction, it is included at `main.c`.
