# Monte Carlo Tree Search
## Compile
```g++ main.cpp state.cpp mcts.cpp -o MCTS```
## Run
### Arguments
3 arguments to be set\
```./MCTS -?``` for help
![](https://i.imgur.com/S0rI2cZ.png)

* -m:
    * manual mode:由我們手動輸入action,與MCTS對弈一場
    * auto mode:由隨機選擇action,與MCTS對弈數場(-n 設定場數)
* -n:
    總共要對弈的場數
* -i:
    MCTS要跑的simulation次數