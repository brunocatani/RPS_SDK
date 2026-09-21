# Input and player controller

Input and PlayerController are separately discovered interfaces. Use [Input](modular/Input.md) for raw/logical input and suppression, and [PlayerController](modular/PlayerController.md) for copied controller state and guarded native jump admission.

PlayerController does not expose penetration checks, arbitrary movement, teleportation or a native pointer. A jump request needs current generations and a finite positive height no greater than 256 game units. Establish clearance in the consumer before requesting it. Offhand reservations belong to [Grab](modular/Grab.md), not Input.
