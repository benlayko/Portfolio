import org.nd4j.linalg.api.ndarray.INDArray;

public class MaxABNode implements ABNode{
	private float value;
	private INDArray state;
	private ABNode successor;
	private int[] change;
	
	public MaxABNode(INDArray board, int[] change, ABNode successor, boolean terminal, final float fvalue) {
		state = board;
		this.change = change;
		this.successor = successor;
		

		value = fvalue;
	}
	
	public float getValue() {
		return value;
	}
	
	public INDArray getBoard() {
		return state;
	}
	
	public ABNode getSuccessor(){
		return successor;
	}
	
	public int[] getChange() {
		return change;
	}
}