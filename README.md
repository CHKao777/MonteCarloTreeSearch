# Monte Carlo Tree Search
## Compile
```make``` to generate MCTS executable
## Run
### Arguments
```./MCTS --help``` for help
![](https://i.imgur.com/tDyE9u2.png)
### Examples:
```./MCTS --serial --root --iter 100 --game_num 1000 ```  
serial vs. root, total # of simulations for next step = 100, total # of games = 1000  
```./MCTS --tree --leaf --ms 50 --game_num 100 ```  
tree vs. leaf, thinking time for next step = 50 milliseconds, total # of games = 100  
