import org.nd4j.linalg.api.buffer.DataType;
import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.factory.Nd4j;

import java.util.*;
import java.io.File;
import java.io.FileNotFoundException;

public class TicTacToe {
	private float probability;
	private int mode;
	
	public INDArray load_board(){
		INDArray board = null;
		
		try{
			File input_file = new File("Input");
			Scanner reader = new Scanner(input_file);
			
			//load dimension and length of board
			String tmp = reader.nextLine();
			String[] tmp2 = tmp.split(" ");
			this.mode = Integer.parseInt(tmp2[0]);
			int dimension = Integer.parseInt(tmp2[1]);
			int length = Integer.parseInt(tmp2[2]);
			
			int [] board_shape = new int[dimension];
			for (int i=0; i<board_shape.length; i++){
				board_shape[i] = length;
			}
			
			board = Nd4j.create(board_shape, DataType.INT16);
			
			//load probability of action success
			String prob_str = reader.nextLine();
			this.probability = Float.parseFloat(prob_str);
			
			//load first player's already drawn tiles
			String [] tmp_input;
			int [] indices;
			int first_num_move = Integer.parseInt(reader.nextLine());
			for (int i=0; i<first_num_move; i++){
				tmp_input = reader.nextLine().split(" ");
				indices = new int[tmp_input.length];
				for (int j=0; j<tmp_input.length; j++){
					indices[j] = Integer.parseInt(tmp_input[j]);
				}
				board.putScalar(indices, 1);
			}
			
			//load second player's already drawn tiles
			int second_num_move = Integer.parseInt(reader.nextLine());
			for (int i=0; i<second_num_move; i++){
				tmp_input = reader.nextLine().split(" ");
				indices = new int[tmp_input.length];
				for (int j=0; j<tmp_input.length; j++){
					indices[j] = Integer.parseInt(tmp_input[j]);
				}
				board.putScalar(indices, 2);
			}
			
		} catch (FileNotFoundException e){
			e.printStackTrace();
		}
		
		
		return board;
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
	
	public int check_winner(INDArray board){
		INDArray first_filled = board.eq(1);
		INDArray second_filled = board.eq(2);
		
		if (this.check_if_winning(first_filled)){
			return 1;
		}else if (this.check_if_winning(second_filled)){
			return 2;
		}else{
			first_filled = first_filled.castTo(DataType.INT8);
			second_filled = second_filled.castTo(DataType.INT8);
			INDArray combined = first_filled.add(second_filled);
			int total_filled = combined.sum().getInt(0);
			int board_size = this.compute_board_size(board);
			
			if (total_filled == board_size){
				return -1;
			}else{
				return 0;
			}
		}
	}
	
	public INDArray perform_action(int player, int [] loc, INDArray board){
		int mark = player;
		
		int current_val = board.getInt(loc);
		if (current_val == 1 || current_val == 2){
			throw new IllegalStateException(String.format("Tile at %s is already filled with %d.", Arrays.toString(loc), current_val));
		}
		
		if (this.probability < 1.0 && this.probability > 0.0){
			INDArray valid_locs = board.eq(0);
			ArrayList<int[]> valid_indices = MathHelper.ArgWhere(valid_locs);
			
			//remove selected location
			int dup_loc_ind = -1;
			for (int i=0; i<valid_indices.size(); i++){
				if (Arrays.equals(valid_indices.get(i), loc)){
					dup_loc_ind = i;
				}
			}
			valid_indices.remove(dup_loc_ind);
			
			double rand = Math.random();
			if (rand <= probability || valid_indices.size() == 0){
				board.putScalar(loc, mark);
			}else{
				Random rand_gen = new Random();
				int tar = rand_gen.nextInt(valid_indices.size());
				int [] new_loc = valid_indices.get(tar);
				board.putScalar(new_loc, mark);
			}
			
		}else{
			board.putScalar(loc, mark);
		}
		
		return board;
	}
	
	public float getProb() {
		return probability;
	}
	public static void main(String [] args){
		//initialize game and load the board from the sepecification.
		TicTacToe game = new TicTacToe();
		INDArray board = game.load_board();
		
		//initialize players;
		Player player1 = new Player(1,  game.mode, game.getProb());
		Player player2 = new Player(2,game.mode, game.getProb() );
		Player [] players = {player1, player2};
		int cur_player = game.check_next_player(board);
		
		//planning the game tree and estimate the score
		int estimated_score1 = player1.construct_game_tree(board);
		int estimated_score2 = player2.construct_game_tree(board);
		System.out.println(board);
		System.out.println("################");
		
		//compute max iteration of the game
		int max_size = game.compute_board_size(board);
		
		//main iteration of the game
		int winner = 0;
		Player player;
		int[] loc;
		INDArray previous_board = null;
		for (int i=0; i<max_size; i++){
			winner = game.check_winner(board);
			if(winner != 0){
				break;
			}
			
			player = players[cur_player-1];
			loc = player.getAction(board, previous_board);
			previous_board = board.dup();
			board = game.perform_action(player.getPlayerId(), loc, board);
			
			System.out.println(board);
			System.out.println("################");
			
			cur_player = game.check_next_player(board);
		}
		
		if(winner != -1){
			System.out.println(String.format("Final winner: player %d", winner));
		}else{
			System.out.println("Draw");
		}
		
		player1.print_game_tree_node_num();
		System.out.println(String.format("Player 1 estimated: %d", estimated_score1));
		player2.print_game_tree_node_num();
		System.out.println(String.format("Player 2 estimated: %d", estimated_score2));
	}
	
}
