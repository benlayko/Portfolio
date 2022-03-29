import java.util.ArrayList;

import org.nd4j.linalg.api.ndarray.INDArray;

public class MaxNode implements Node{
	private float value;
	private INDArray state;
	private ArrayList<Node> successors;
	private int[] change;
	private boolean term;
	
	public MaxNode(INDArray board, int[] change, ArrayList<Node> successors, boolean terminal, final float fvalue) {
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
