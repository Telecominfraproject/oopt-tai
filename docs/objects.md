## Example Platform

- Facebook Voyager
    - Fixed 4 optical line ports
    - 2 Acacia AC400 (Dual Channel Module)
- Edgecore Cassini
    - 8 plug-in-unit(PIU) slots
    - 3 types of PIU
        1. CFP2ACO PIU
        2. CFP2DCO PIU
        3. QSFP28 PIU ( out of scope for TAI )

## module, network-interface, host-interface relationship

|                     | module | network interface                               | host interface
|---------------------|--------|-------------------------------------------------|----
| AC400               | 1      | 2 = ( 1 Tunable Laser + 1 Channel of DSP ) * 2  | 4
| Cassini CFP2ACO PIU | 1      | 1 = CFP2ACO + DSP                               | 2
| Cassini CFP2DCO PIU | 1      | 1 = CFP2DCO                                     | 2
