import java.util.ArrayList;
import java.util.Arrays;
import java.util.Random;

import org.nd4j.linalg.api.ndarray.INDArray;

public class MinNode implements Node{
	private float value;
	private INDArray state;
	private ArrayList<Node> successors;
	private int[] change;
	private boolean term;
	
	public MinNode(INDArray board, int[] change,ArrayList<Node> successors, boolean terminal, final float fvalue) {
		state = board;
		this.change = change;
		this.successors = successors;
		this.term = terminal;

		value = fvalue;

	}
	
	public float getValue() {
		return value;
	}
	
	public INDArray getBoard() {
		return state;
	}
	
	public ArrayList<Node> getSuccessors(){
		if(successors == null) {
			return new ArrayList<Node>();
		}
		
		return successors;
	}
	
	public int[] getChange() {
		return change;
	}
	
	public boolean terminal() {
		return term;
	}
	
}
