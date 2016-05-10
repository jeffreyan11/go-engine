# go-engine
An engine for playing the board game Go.

### Implementation
This is a Monte-Carlo tree search based engine, with very basic heuristics for the playout.

### Play strength
| Days since birth (Jan 22, 2016) | Elo (1000 playouts, 0 elo is random mover) | Net Speed Difference |
|:-------------------------------:|:------------------------------------------:|:--------------------:|
|                41               |                    28.6                    |         100%         |
|                47               |                    39.7                    |        150.2%        |
|                50               |                    310.2                   |        140.7%        |
|                63               |                    506.0                   |        134.8%        |
|                90               |                    505.7                   |        202.5%        |
|                95               |                   1366.4                   |        246.4%        |
|               105               |                   1607.2                   |         87.6%        |

### Acknowledgements
A special thanks to Donnie Pinkston (Caltech), who has given me valuable advice throughout this project.