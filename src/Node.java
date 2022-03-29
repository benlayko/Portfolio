import java.util.ArrayList;

import org.nd4j.linalg.api.ndarray.INDArray;

public interface Node {
	public float getValue();
	public INDArray getBoard();
	public ArrayList<Node> getSuccessors();
	public int[] getChange();
	public boolean terminal();
}
