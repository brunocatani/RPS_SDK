# Choose the owning interface

Start with the runtime owner of the behavior. Bind Core Read for lifecycle observations and Core Callbacks only when you need scheduled frame/phase work.

| Feature | Interfaces | Boundary |
| --- | --- | --- |
| Hand/HMD observer | Hands Read | Final poses at Core Presented; validity and generations required. |
| World pickup or consumable/throwable inventory handoff | Grab Read + Write | Queue a command and observe its terminal state; inventory handoff chooses an available hand. |
| Holster draw or inventory weapon switch | Weapon 1.1 Read + Write, Core scheduling | Capture the exact stack, choose the physical hand, and wait for the terminal result; retain an opposite-hand weapon with Toggle Drop controls. |
| Climbing or physical controls | Touch Read + Write, Grab Read; Collision as needed | Scoped target leases and explicit clear/yield cleanup. |
| Weapon inspection | Weapon and WeaponParts Read | Keep weapon-generation and source keys together. |
| Part manipulation | WeaponParts Read + Write | Separate target matching, drive publication and application readback. |
| Input gesture | Input Read; Write for suppression | Physical hand identity and raw levels are separate from weapon roles. |
| Native pose coordination | Animation Read + Write, Core callbacks | Narrow leased authority; clear before owner teardown. |
| Power Armor attachment | References Read, Grab Read + Write | Classification and animated anchor validation precede command admission. |
| External physics sensor | Collision Read + Write | Consumer owns body lifetime; clear scopes before destroying bodies. |
| Controller/jump observation | PlayerController Read; Write for jump | Native guarded jump only, no arbitrary locomotion or penetration API. |
| Diagnostic overlay | Diagnostics Write | Bounded copied data and expiring publications. |
| Configuration panel | Configuration Read + Write, optional RPS UI | Use the compiled catalog and ROCK's persistence service. |

A native operation outside these contracts remains your plugin's responsibility. [RPS Framework](https://devartificial.pro/docs/rps-framework) can support owned engine integration; a source key or body ID does not grant arbitrary native access.
