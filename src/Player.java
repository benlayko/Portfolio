import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

import org.nd4j.linalg.api.buffer.DataType;
import org.nd4j.linalg.api.ndarray.INDArray;

public class Player {
	private int player_id;
	private int other_id;
	private int mode;
	private float probability;
	private int num_tree_node=0;
	//Used for all
	private Node placeHolder;
	//Used to keep track of the game tree for a player in game mode 0
	private Node root;
	private Node current;
	//Used to keep track of the game tree for a player in game mode 1
	private ABNode abroot;
	private ABNode abcurrent;
	private float alpha = Float.NEGATIVE_INFINITY;
	private float beta = Float.POSITIVE_INFINITY;
	//Used to keep track of the game tree for a player in game mode 2
	private int depthLimit;
	private Node rootd;
	private Node currentd;
	boolean first = true;
	
	public Player(int player_id, int mode, float probability){
		//Assume player 1 goes first and player 2 goes second.
		this.player_id = player_id;
		this.mode = mode;
		this.probability = probability;
	}
	
	public int getPlayerId(){
		return this.player_id;
	}
	
	public int construct_game_tree(final INDArray board){
		//Construct your game tree based on given board condition here.
		//Return the estimated score.
		int estimated_score = 0;
		
		//Construct the game tree based on specified mode.
		if (this.mode == 0){
			//This section is used for the minimax algorithm. I needed to put this in this section rather than the method so that I could keep track of the change
			int boardValue = check_winner(board);			
			//Check if the board is in a terminal state
			if(boardValue == -20) {
				
				//Gets all the locations that are playable
				INDArray valid_locs = board.eq(0);
				ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
				ArrayList<Node> successors = new ArrayList<Node>();
					
				//Iterates through the locations and generates the min values
				for(int i = 0; i < valid_indices.size(); i++) {
					INDArray newState = board.dup();
					newState.putScalar(valid_indices.get(i), 1);
					successors.add(generateMin(newState, valid_indices.get(i)));
				}
				
				//This finds the max of all the min nodes
				float max = -200;
				float avgVal = 0;
				for(Node n: successors) {
					if(n.getValue() > max) {
						max = n.getValue();
						continue;
					}
					//This is used for the nondeterministic case
					avgVal += n.getValue() * ((1 - this.probability));
				}
				if(successors.size() > 1) {
					avgVal = avgVal / (successors.size() - 1);
				}
				avgVal += max * this.probability;
				//Create the max node
				root = new MaxNode(board, null, successors, false, avgVal);
				num_tree_node++;
			} else {
				root = new MaxNode(board, null, null, true, boardValue);
				num_tree_node++;
			}
			
			current = root;
			estimated_score = (int)root.getValue();
		
		
		
		
		}else if (this.mode == 1){
			//Calls the alpha beta method
			abroot = maxValue(board, null);
			abcurrent = abroot;
			estimated_score = (int) abroot.getValue();
		
		
		
		
		}else if (this.mode == 2){
			//This is similar to the minimax, but with a depth limit. It also checks which player is making the tree to alternate starting with a min or max node
			this.depthLimit = 5;
			if(this.player_id == 1) {
				int boardValue = check_winner(board);			
				if(boardValue == -20) {
					
					INDArray valid_locs = board.eq(0);
					ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
					ArrayList<Node> successors = new ArrayList<Node>();
					
					for(int i = 0; i < valid_indices.size(); i++) {
						INDArray newState = board.dup();
						newState.putScalar(valid_indices.get(i), 1);
						successors.add(generateMinWithDepth(newState, valid_indices.get(i), 0));
					}	
				
					float max = -200;
					float avgVal = 0;
					for(Node n: successors) {
						if(n.getValue() > max) {
							max = n.getValue();
							continue;
						}
						avgVal += n.getValue() * ((1 - this.probability));
					}
					if(successors.size() > 1) {
						avgVal = avgVal / (successors.size() - 1);
					}
					avgVal += max * this.probability;
					rootd = new MaxNode(board, null, successors, false, avgVal);
					num_tree_node++;
				} else {
					rootd = new MaxNode(board, null, null, true, boardValue);
					num_tree_node++;
				}
				estimated_score = (int) rootd.getValue();
			}
			else {
				int boardValue = check_winner(board);			
				if(boardValue == -20) {
					
					INDArray valid_locs = board.eq(0);
					ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
					ArrayList<Node> successors = new ArrayList<Node>();
					
					for(int i = 0; i < valid_indices.size(); i++) {
						INDArray newState = board.dup();
						newState.putScalar(valid_indices.get(i), 1);
						successors.add(generateMaxWithDepth(newState, valid_indices.get(i), 0));
					}	
				
					float min = 200;
					float avgVal = 0;
					for(Node n: successors) {
						if(n.getValue() < min) {
							min = n.getValue();
							continue;
						}
						avgVal += n.getValue() * ((1 - this.probability));
					}
					if(successors.size() > 1) {
						avgVal = avgVal / (successors.size() - 1);
					}
					avgVal += min * this.probability;
					rootd = new MinNode(board, null, successors, false, avgVal);
					num_tree_node++;
				} else {
					rootd = new MinNode(board, null, null, true, boardValue);
					num_tree_node++;
				}
			}
			currentd = rootd;
			estimated_score = (int)rootd.getValue();
			System.out.println(num_tree_node);
		}else{
			throw new IllegalStateException(String.format("Invalid player mode: %d.", this.mode));
		}
		
		//Estimate the total number of nodes in the game tree here.
		
		return estimated_score;
	}
	
	public MaxNode generateMax(final INDArray board, int[] change) {
		int boardValue = check_winner(board);
		//Check if the board is in a terminal state
		if(boardValue == -20) {
			//If it is not find all valid locations
			INDArray valid_locs = board.eq(0);
			ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
			ArrayList<Node> successors = new ArrayList<Node>();
			//Iterate through the valid locations and generate the successor min nodes
			for(int i = 0; i < valid_indices.size(); i++) {
				INDArray newState = board.dup();
				newState.putScalar(valid_indices.get(i), 1);
				successors.add(generateMin(newState, valid_indices.get(i)));
			}
			//Find the largest min node to place as this nodes value
			float max = -200;
			float avgVal = 0;
			for(Node n: successors) {
				if(n.getValue() > max) {
					max = n.getValue();
					continue;
				}
				//Used for non-determinisic case
				avgVal += n.getValue() * ((1 - this.probability));
			}
			if(successors.size() > 1) {
				avgVal = avgVal / (successors.size() - 1);
			}
			avgVal += max * this.probability;
			//Create new max nodde
			MaxNode nonTerminal = new MaxNode(board, change, successors, false, avgVal);
			num_tree_node++;
			return nonTerminal;
		} else {
			//If the node is terminal return its value
			MaxNode terminal = new MaxNode(board, change, null, true, boardValue);
			num_tree_node++;
			System.out.println(num_tree_node);
			return terminal;
		}
	}
	
	public MinNode generateMin(final INDArray board, int[] change) {
		int boardValue = check_winner(board);
		//See generate max for walk through of how this works
		if(boardValue == -20) {
			
			INDArray valid_locs = board.eq(0);
			ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
			ArrayList<Node> successors = new ArrayList<Node>();
			
			for(int i = 0; i < valid_indices.size(); i++) {
				INDArray newState = board.dup();
				newState.putScalar(valid_indices.get(i), 2);
				successors.add(generateMax(newState, valid_indices.get(i)));
			}
			float min = 200;
			float avgVal = 0;
			for(Node n: successors) {
				if(n.getValue() < min) {
					min = n.getValue();
					continue;
				}
				avgVal += n.getValue() * ((1 - this.probability));
			}
			if(successors.size() > 1) {
				avgVal = avgVal / (successors.size() - 1);
			}
			avgVal += min * this.probability;
			MinNode nonTerminal = new MinNode(board, change, successors, false, avgVal);
			num_tree_node++;
			return nonTerminal;
		} else {
			MinNode terminal = new MinNode(board, change, null, true, boardValue);
			num_tree_node++;
			return terminal;
		}
	}
	
	public MaxABNode maxValue(INDArray board, int[] change) {
		float state = (float) check_winner(board);
		//Checks if the node is terminal
		if(state != -20) {
			num_tree_node++;
			return new MaxABNode(board, change, null, true, state);
		}
		//If it is not create a new alpha beta max node with starting value
		MaxABNode v = new MaxABNode(board, change, null, false, Float.NEGATIVE_INFINITY);
		num_tree_node++;
		System.out.println(num_tree_node);
		
		//Find valid locations
		INDArray valid_locs = board.eq(0);
		ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
		//Iterate through valid locations
		for(int i = 0; i < valid_indices.size(); i++) {
			INDArray newState = board.dup();
			newState.putScalar(valid_indices.get(i), 1);
			//Call the minValue function 
			MinABNode x = minValue(newState, valid_indices.get(i));
			num_tree_node++;
			if(x.getValue() > v.getValue()) {
				//Test to see if the new node should replace v
				v = new MaxABNode(newState, change, x, false, x.getValue());
			}
			
			if(v.getValue() > beta) {
				return v;
			}
			alpha = Math.max(alpha, v.getValue());
		}
		return v;
	}
	
	public MinABNode minValue(INDArray board, int[] change) {
		//See maxValue for explanation of code
		int state = check_winner(board);
		if(state != -20) {
			num_tree_node++;
			return new MinABNode(board, change, null, true, state);
		}
		MinABNode v = new MinABNode(board, change, null, false, Float.POSITIVE_INFINITY);
		num_tree_node++;
		INDArray valid_locs = board.eq(0);
		ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
		
		for(int i = 0; i < valid_indices.size(); i++) {
			INDArray newState = board.dup();
			newState.putScalar(valid_indices.get(i), 2);
			MaxABNode x = maxValue(newState, valid_indices.get(i));
			num_tree_node++;
			if(x.getValue() < v.getValue()) {
				v = new MinABNode(newState, change, x, false, x.getValue());
			}
			
			
			if(v.getValue() < alpha) {
				return v;
			}
			beta = Math.min(beta, v.getValue());
		}
		return v;
	}
	
	public MaxNode generateMaxWithDepth(final INDArray board, int[] change, int depth) {
		//This is the same as generateMax, but it also has a parameter to limit the depth the game tree can go
		int boardValue = check_winner(board);
		
		if(boardValue == -20 && depth < this.depthLimit) {
			
			INDArray valid_locs = board.eq(0);
			ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
			ArrayList<Node> successors = new ArrayList<Node>();
			
			for(int i = 0; i < valid_indices.size(); i++) {
				INDArray newState = board.dup();
				newState.putScalar(valid_indices.get(i), 1);
				successors.add(generateMinWithDepth(newState, valid_indices.get(i), depth + 1));
			}
			
			float max = -200;
			float avgVal = 0;
			for(Node n: successors) {
				if(n.getValue() > max) {
					max = n.getValue();
					continue;
				}
				avgVal += n.getValue() * ((1 - this.probability));
			}
			if(successors.size() > 1) {
				avgVal = avgVal / (successors.size() - 1);
			}
			avgVal += max * this.probability;
			
			MaxNode nonTerminal = new MaxNode(board, change, successors, false, avgVal);
			num_tree_node++;
			return nonTerminal;
		}  else if(depth == depthLimit) {
			//If the depth limit has been reached, make the value of the node 0 and return
			MaxNode terminal = new MaxNode(board, change, null, true, 0);
			num_tree_node++;
			//System.out.println(num_tree_node);
			return terminal;
		} else {
			MaxNode terminal = new MaxNode(board, change, null, true, boardValue);
			num_tree_node++;
			//System.out.println(num_tree_node);
			return terminal;
		}
	}
	
	public MinNode generateMinWithDepth(final INDArray board, int[] change, int depth) {
		int boardValue = check_winner(board);
		if(boardValue == -20 && depth < depthLimit) {
			
			INDArray valid_locs = board.eq(0);
			ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
			ArrayList<Node> successors = new ArrayList<Node>();
			
			for(int i = 0; i < valid_indices.size(); i++) {
				INDArray newState = board.dup();
				newState.putScalar(valid_indices.get(i), 2);
				successors.add(generateMaxWithDepth(newState, valid_indices.get(i), depth + 1));
			}
			float min = 200;
			float avgVal = 0;
			for(Node n: successors) {
				if(n.getValue() < min) {
					min = n.getValue();
					continue;
				}
				avgVal += n.getValue() * ((1 - this.probability));
			}
			if(successors.size() > 1) {
				avgVal = avgVal / (successors.size() - 1);
			}
			avgVal += min * this.probability;
			MinNode nonTerminal = new MinNode(board, change, successors, false, avgVal);
			num_tree_node++;
			return nonTerminal;
		} else if(depth == depthLimit) {
			MinNode terminal = new MinNode(board, change, null, true, 0);
			num_tree_node++;
			return terminal;
		} else {
			MinNode terminal = new MinNode(board, change, null, true, boardValue);
			num_tree_node++;
			return terminal;
		}
	}
	
	public void print_game_tree_node_num(){
		System.out.println(String.format("Player %d number of noded in the tree: %d", this.player_id, this.num_tree_node));
		return;
	}
	
	public int[] randomSampleAction(final INDArray board){
		//for debug purpose only, don't use in actual implementation
		INDArray valid_locs = board.eq(0);
		ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
		
		Random rand_gen = new Random();
		int tar = rand_gen.nextInt(valid_indices.size());
		int [] new_loc = valid_indices.get(tar);
		return new_loc;
		
	}
	
	public int[] getAction(final INDArray current_board, final INDArray previous_board){
		//Replace the following line with the target location of decided by the algorithm and the game tree.
		int [] loc = this.randomSampleAction(current_board);
		
		//Select action based on constructed game tree.
		if (this.mode == 0){
			if(this.probability == 1) {
				//Checks which player is going
				if(this.player_id == 1) {
					//If the first player has not gone yet this finds the best action
					if(current.equals(root)) {
						float score = -200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() > score) {
								score = n.getValue();
								loc = n.getChange();
								current = n;
							}
						}
					} else {
						//If the first player has gone this advances the game board by matching the current board with the current nodes successors
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(current_board.eq(1)) && n.getBoard().eq(2).equals(current_board.eq(2))) {
								current = n;
								break;
							}
						}
					
						//Using the new current the board finds the best move
						float score = -200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() > score) {
								score = n.getValue();
								loc = n.getChange();
								current = n;
							}
						}
					}
				} else {
				
					//Finds the current node based on the move the first player made and the current board
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(current_board.eq(1)) && n.getBoard().eq(2).equals(current_board.eq(2))) {
								current = n;
								break;
							}
						}
				
						//Finds the best choice for the player
						float score = 200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() < score) {
								score = n.getValue();
								loc = n.getChange();
								current = n;
							}
						}
				}
			} else {
				//This handles the non deterministic case
				if(this.player_id == 1) {
					if(first) {
						float score = -200;
						//Makes the first move
						for(Node n: current.getSuccessors()) {
							if(n.getValue() > score) {
								score = n.getValue();
								loc = n.getChange();
							}
						}
						first = false;
					} else {
						// Because actions are not deterministic the player has to check what the last two moves were
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(previous_board.eq(1)) && n.getBoard().eq(2).equals(previous_board.eq(2))) {
								current = n;
								break;
							}
						}
						
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(current_board.eq(1)) && n.getBoard().eq(2).equals(current_board.eq(2))) {
								current = n;
								break;
							}
						}
						//Choses the best move here
						float score = -200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() > score) {
								score = n.getValue();
								loc = n.getChange();
							}
						}
					}
				} else {
				
					if(first) {
						//If it is the first turn player two must find the correct node
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(current_board.eq(1)) && n.getBoard().eq(2).equals(current_board.eq(2))) {
								current = n;
								break;
							}
						}
						//Makes the best choice here
						float score = 200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() < score) {
								score = n.getValue();
								loc = n.getChange();
							}
						}
						first = false;
					} else {
					
						//Figures out what the last two moves that were made here
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(previous_board.eq(1)) && n.getBoard().eq(2).equals(previous_board.eq(2))) {
								current = n;
								break;
							}
						}
						for(Node n: current.getSuccessors()) {
							if(n.getBoard().eq(1).equals(current_board.eq(1)) && n.getBoard().eq(2).equals(current_board.eq(2))) {
								current = n;
								break;
							}
						}
				
						//Finds the best move
						float score = 200;
						for(Node n: current.getSuccessors()) {
							if(n.getValue() < score) {
								score = n.getValue();
								loc = n.getChange();
							}
						}
					}
				}
			}
		}else if (this.mode == 1){
			if(this.player_id == 1) {
				//Goes over the game tree, but skips steps of the other player
				loc = abcurrent.getSuccessor().getChange();
				abcurrent = abcurrent.getSuccessor();
				abcurrent = abcurrent.getSuccessor();
			} else {
				abcurrent = abcurrent.getSuccessor();
				loc = abcurrent.getSuccessor().getChange();
				abcurrent = abcurrent.getSuccessor();
			}
		}else if (this.mode == 2){
			if(this.player_id == 1) {
				//If it is not the first move, remake the game tree
				if(!first) {
					construct_game_tree(current_board);
				} 
				float score = -200;
				//Find the best move of the current node
				for(Node n: currentd.getSuccessors()) {
					if(n.getValue() > score) {
						score = n.getValue();
						if(n.terminal()) {
							score--;
						}
						loc = n.getChange();
						placeHolder = n;
					}
				}
				//Iterate the node
				currentd = placeHolder;
				first = false;
			} else {
				//Construct a new game tree
				construct_game_tree(current_board);
				float score = 200;
				//Find the best move
				for(Node n: currentd.getSuccessors()) {
					if(n.getValue() < score) {
						score = n.getValue();
						if(n.terminal()) {
							score++;
						}
						loc = n.getChange();
						placeHolder = n;
					}
				}
				currentd = placeHolder;
			}
		}else{
			throw new IllegalStateException(String.format("Invalid player mode: %d.", this.mode));
		}
		
		return loc;
	}
	
	//Changed the values so that min max can be used more easily
	public int check_winner(INDArray board){
		INDArray first_filled = board.eq(1);
		INDArray second_filled = board.eq(2);
		
		if (this.check_if_winning(first_filled)){
			return 1;
		}else if (this.check_if_winning(second_filled)){
			return -1;
		}else{
			first_filled = first_filled.castTo(DataType.INT8);
			second_filled = second_filled.castTo(DataType.INT8);
			INDArray combined = first_filled.add(second_filled);
			int total_filled = combined.sum().getInt(0);
			int board_size = this.compute_board_size(board);
			
			if (total_filled == board_size){
				return 0;
			}else{
				return -20;
			}
		}
	}
	public boolean check_if_winning(INDArray bool_filled){
		INDArray filled = bool_filled.castTo(DataType.INT8);
		long[] board_shape = filled.shape();
		int winning = (int) board_shape[0];
		int num_dim = board_shape.length;
		
		boolean is_winning;
		//check all straight lines
		INDArray filled_len;
		for (int i=0; i<num_dim; i++){
			filled_len = filled.sum(i);
			is_winning = filled_len.eq(winning).any();
			if (is_winning){
				return true;
			}
		}
		
		//check diagonals
		int ax1;
		int ax2;
		for (int i=0; i<num_dim; i++){
			ax1 = i;
			ax2 = (i+1)%num_dim;
			
			filled_len = MathHelper.Trace(filled, ax1, ax2);
			is_winning = filled_len.eq(winning).any();
			if (is_winning){
				return true;
			}
			
			filled_len = MathHelper.ReverseTrace(filled, ax1, ax2);
			is_winning = filled_len.eq(winning).any();
			if (is_winning){
				return true;
			}
		}
		
		//check 3-D diagonal
		if (num_dim == 3){
			int[] start_index = {0, 0, 0};
			
			int diag_trace = 0;
			for(int i = 0; i<winning; i++){
				int[] cur_index = {0, 0, 0};
				for(int j=0; j<num_dim; j++){
					cur_index[j] += i;
				}
				diag_trace += filled.getInt(cur_index);
			}
			
			is_winning = diag_trace == winning;
			
			for(int i=0; i<num_dim; i++){
				start_index = new int[]{0, 0, 0};
				start_index[i] = winning-1;
				
				diag_trace = 0;
				for(int j=0; j<winning; j++){
					int[] cur_index = {start_index[0], start_index[1], start_index[2]};
					for (int c=0; c<num_dim; c++){
						int added = 1;
						if (start_index[c] == 0){
							added = 1;
						}else if (start_index[c] == winning-1){
							added = -1;
						}
						cur_index[c] += j*added;
					}
					diag_trace += filled.getInt(cur_index);
				}
				
				is_winning = is_winning || diag_trace == winning;
			}
			
			if (is_winning){
				return true;
			}
		}
		
		return false;
	}
	public int compute_board_size(INDArray board){
		int max_size = 1;
		long[] board_shape = board.shape();
		for (int i=0; i<board_shape.length; i++){
			max_size *= board_shape[i];
		}
		return max_size;
	}
	public int check_next_player(INDArray board){
		INDArray first_filled = board.eq(1).castTo(DataType.INT8);
		INDArray second_filled = board.eq(2).castTo(DataType.INT8);
		
		int filled_1 = first_filled.sum().getInt(0);
		int filled_2 = second_filled.sum().getInt(0);
		
		if (filled_1 != filled_2 && filled_1 != filled_2+1){
			throw new IllegalStateException(String.format("The board is inconsistent: %d %d", filled_1, filled_2));
		}else if(filled_1 == filled_2){
			return 1;
		}else if(filled_1 == filled_2 + 1){
			return 2;
		}
		
		return -1;
	}
}
