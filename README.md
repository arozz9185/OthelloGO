# OthelloGO
an AI example for playing Othello(reversi)

In the src package above you will see a main.cpp which is a designed egine that runs othello.
You can include two executable file into main.cpp to play the game with AI you design. You can try your AI out with the provieded baseline test.

In this particular example, the AI is based on MiniMax Tree Search, State Value Function and Alpha-beta Pruning.
And the theory is quite simple:                                                                  
  1.MiniMax Tree: Use a basic binary tree search to simulate the possible outcome and try to find the target state value each player want most.   
  2.State Value Fuction: Define a function that generate the value of each state given a Othello Board.                                         
  3.Alpha-Beta Pruning: One of the simple way to optimize and accelerate the process of tree search. Pruning the branches that is unecessary. 
                                                                                                                                                  
 References:  https://www.othelloonline.org/, https://en.wikipedia.org/wiki/Minimax, https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning.
