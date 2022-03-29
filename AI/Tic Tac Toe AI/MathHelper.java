import java.util.ArrayList;

import org.nd4j.linalg.api.ndarray.INDArray;
import org.nd4j.linalg.factory.Nd4j;

public class MathHelper {
	public static INDArray Trace(INDArray matrix, int axis1, int axis2){
		if (axis1 == axis2){
			throw new IllegalStateException(String.format("Axis 1 must not be the same as axis 2: %d vs. %d", axis1, axis2));
		}
		
		long[] board_shape = matrix.shape();
		int winning = (int) board_shape[0];
		int num_dim = board_shape.length;
		
		INDArray eye_mask = Nd4j.eye(winning);
		
		int[] permut_index = new int[num_dim];
		permut_index[0] = axis1;
		permut_index[1] = axis2;
		int permut_i=2;
		for(int i=0; i<num_dim; i++){
			if (i != axis1 && i != axis2){
				permut_index[permut_i] = i;
				permut_i++;
			}
		}
		
		INDArray new_matrix = matrix.permute(permut_index);
		new_matrix = new_matrix.mul(eye_mask);
		new_matrix = new_matrix.sum(num_dim-2, num_dim-1);
		
		return new_matrix;
	}
	
	public static INDArray ReverseTrace(INDArray matrix, int axis1, int axis2){
		if (axis1 == axis2){
			throw new IllegalStateException(String.format("Axis 1 must not be the same as axis 2: %d vs. %d", axis1, axis2));
		}
		
		long[] board_shape = matrix.shape();
		int winning = (int) board_shape[0];
		int num_dim = board_shape.length;
		
		INDArray eye_mask = ReverseEye(winning);
		
		int[] permut_index = new int[num_dim];
		permut_index[0] = axis1;
		permut_index[1] = axis2;
		int permut_i=2;
		for(int i=0; i<num_dim; i++){
			if (i != axis1 && i != axis2){
				permut_index[permut_i] = i;
				permut_i++;
			}
		}
		
		INDArray new_matrix = matrix.permute(permut_index);
		new_matrix = new_matrix.mul(eye_mask);
		new_matrix = new_matrix.sum(num_dim-2, num_dim-1);
		
		return new_matrix;
	}
	
	public static INDArray ReverseEye(int size){
		INDArray reverse_eye = Nd4j.zeros(size, size);
		
		for(int i=0; i<size; i++){
			int[] ind = {size-i-1, i};
			reverse_eye.putScalar(ind, 1.0);
		}
		
		return reverse_eye;
	}
	
	public static ArrayList ArgWhere(INDArray where){
		ArrayList<int[]> indices = new ArrayList<int[]>();
		long[] shape = where.shape();
		
		int[] cur_index = new int[shape.length];
		long total_length = 1;
		for(int i=0; i<cur_index.length; i++){
			cur_index[i] = 0;
			total_length *= shape[i];
		}
		
		boolean cond;
		for(int i=0; i<total_length; i++){
			cond = where.getInt(cur_index) > 0;
			if(cond){
				indices.add(cur_index.clone());
			}
			
			//update cur_index
			int carry=0;
			cur_index[0] += 1;
			if (cur_index[0] >= shape[0]){
				cur_index[0] = 0;
				carry = 1;
			}
			for (int j=1; j<cur_index.length; j++){
				cur_index[j] += carry;
				if (cur_index[j] >= shape[j]){
					cur_index[j] = 0;
					carry = 1;
				}else{
					carry = 0;
				}
			}
		}
		
		return indices;
	}
}
