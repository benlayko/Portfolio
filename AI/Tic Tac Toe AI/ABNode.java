import org.nd4j.linalg.api.ndarray.INDArray;

public interface ABNode {
	public float getValue();
	public INDArray getBoard();
	public ABNode getSuccessor();
	public int[] getChange();
}
