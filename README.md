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

## Result
[slide](https://docs.google.com/presentation/d/1hHoEAol94RPNg0LN87_MujvCjELHVkZ9PE4xaCaMPAg/edit?usp=share_link)  
[report](https://drive.google.com/file/d/1pzc6UvZfKGjIKni0i5jibDUschiUPEFP/view?usp=share_link)  
