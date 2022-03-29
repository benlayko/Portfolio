import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;

import java.io.IOException;
import java.util.*;

/*
 * Main class of the TFICF MapReduce implementation. Used apache documentation for some help. https://cwiki.apache.org/confluence/display/HADOOP2/WordCount.
 * Author: Ben Layko 
 * Date:   2/28/22
 */
public class TFICF {

    public static void main(String[] args) throws Exception {
        // Check for correct usage
        if (args.length != 2) {
            System.err.println("Usage: TFICF <input corpus0 dir> <input corpus1 dir>");
            System.exit(1);
        }
		
		// return value of run func
		int ret = 0;
		
		// Create configuration
		Configuration conf0 = new Configuration();
		Configuration conf1 = new Configuration();
		
		// Input and output paths for each job
		Path inputPath0 = new Path(args[0]);
		Path inputPath1 = new Path(args[1]);
        try{
            ret = run(conf0, inputPath0, 0);
        }catch(Exception e){
            e.printStackTrace();
        }
        if(ret == 0){
        	try{
            	run(conf1, inputPath1, 1);
        	}catch(Exception e){
            	e.printStackTrace();
        	}        	
        }
     
     	System.exit(ret);
    }
		
	public static int run(Configuration conf, Path path, int index) throws Exception{
		// Input and output paths for each job

		Path wcInputPath = path;
		Path wcOutputPath = new Path("output" +index + "/WordCount");
		Path dsInputPath = wcOutputPath;
		Path dsOutputPath = new Path("output" + index + "/DocSize");
		Path tficfInputPath = dsOutputPath;
		Path tficfOutputPath = new Path("output" + index + "/TFICF");
		
		// Get/set the number of documents (to be used in the TFICF MapReduce job)
        FileSystem fs = path.getFileSystem(conf);
        FileStatus[] stat = fs.listStatus(path);
		String numDocs = String.valueOf(stat.length);
		conf.set("numDocs", numDocs);
		
		// Delete output paths if they exist
		FileSystem hdfs = FileSystem.get(conf);
		if (hdfs.exists(wcOutputPath))
			hdfs.delete(wcOutputPath, true);
		if (hdfs.exists(dsOutputPath))
			hdfs.delete(dsOutputPath, true);
		if (hdfs.exists(tficfOutputPath))
			hdfs.delete(tficfOutputPath, true);
		
		// Create and execute Word Count job
		Job wc =  Job.getInstance(conf, "WordCount");
		//Set all the attributes for the job here
		wc.setJarByClass(TFICF.class);
		wc.setOutputKeyClass(Text.class);
		wc.setOutputValueClass(IntWritable.class);
		wc.setMapperClass(WCMapper.class);
		wc.setReducerClass(WCReducer.class);
		//wc.setInputFormatClass(TextInputFormat.class);
		//wc.setOutputFormatClass(TextOutputFormat.class);
		FileInputFormat.addInputPath(wc, wcInputPath);
		FileOutputFormat.setOutputPath(wc, wcOutputPath);
		wc.waitForCompletion(true);

			
		// Create and execute Document Size job
		
		Job ds = Job.getInstance(conf, "DocSize");
		//Set all the attributes for the job here
		ds.setJarByClass(TFICF.class);
		ds.setOutputKeyClass(Text.class);
		ds.setOutputValueClass(Text.class);
		ds.setMapperClass(DSMapper.class);
		ds.setReducerClass(DSReducer.class);
		//ds.setInputFormatClass(TextInputFormat.class);
		//ds.setOutputFormatClass(TextOutputFormat.class);
		FileInputFormat.addInputPath(ds, dsInputPath);
		FileOutputFormat.setOutputPath(ds, dsOutputPath);
		ds.waitForCompletion(true);
		
		//Create and execute TFICF job
		
		Job tf = Job.getInstance(conf, "TFICF");
		//Set all the attributes for the job here
		tf.setJarByClass(TFICF.class);
		tf.setOutputKeyClass(Text.class);
		tf.setOutputValueClass(Text.class);
		tf.setMapperClass(TFICFMapper.class);
		tf.setReducerClass(TFICFReducer.class);
		//tf.setInputFormatClass(TextInputFormat.class);
		//tf.setOutputFormatClass(TextOutputFormat.class);
		FileInputFormat.addInputPath(tf, tficfInputPath);
		FileOutputFormat.setOutputPath(tf, tficfOutputPath);

		return tf.waitForCompletion(true) ? 0 : 1;
		
    }
	
	/*
	 * Creates a (key,value) pair for every word in the document 
	 *
	 * Input:  ( byte offset , contents of one line )
	 * Output: ( (word@document) , 1 )
	 *
	 * word = an individual word in the document
	 * document = the filename of the document
	 */
	public static class WCMapper extends Mapper<Object, Text, Text, IntWritable> {
		private Text word = new Text();
		private IntWritable one = new IntWritable(1);
		/**
		* Map function of mapper parent class that takes aline of text and returns the count of each word
		*/
		@Override
		protected void map(Object key, Text value, Context context) throws IOException, InterruptedException{
			String textLine = value.toString();
			textLine = textLine.replaceAll("[,.?;!&:`\\[\\]\\(\\)\\{\\}\'\"]", "");
			StringTokenizer st = new StringTokenizer(textLine, " ,.?;!&:`[](){}\n\t");
			String docName = ((FileSplit) context.getInputSplit()).getPath().getName();
			//TO-DO add the document to the key
			while(st.hasMoreTokens()){
				String out = st.nextToken();
				out = out.toLowerCase();
				word.set(out + "@" + docName);
				if(!out.equals("") && out.charAt(0) < 123 && out.charAt(0) > 96){
					context.write(word, one);
				}
			}
		}
		
    }

    /*
	 * For each identical key (word@document), reduces the values (1) into a sum (wordCount)
	 *
	 * Input:  ( (word@document) , 1 )
	 * Output: ( (word@document) , wordCount )
	 *
	 * wordCount = number of times word appears in document
	 */
	public static class WCReducer extends Reducer<Text, IntWritable, Text, IntWritable> {
		
		public void reduce(Text key, Iterable<IntWritable> valueList, Context context)  throws IOException, InterruptedException{
			int sum = 0;
			for(IntWritable value : valueList){
				sum += value.get();
			}
			IntWritable output = new IntWritable(sum);
			context.write(key, output);
		}
		
    }
	
	/*
	 * Rearranges the (key,value) pairs to have only the document as the key
	 *
	 * Input:  ( (word@document) , wordCount )
	 * Output: ( document , (word=wordCount) )
	 */
	public static class DSMapper extends Mapper<Object, Text, Text, Text> {
		private Text doc = new Text();
		private Text word = new Text();
		/**
		* Map function of mapper parent class that takes aline of text and returns the count of each word
		*/
		@Override
		protected void map(Object info, Text value, Context context)  throws IOException, InterruptedException{
			String information = value.toString();
			if(information.contains("@")){
				StringTokenizer st = new StringTokenizer(information);
				String key = st.nextToken();
				String count = st.nextToken();
				doc.set(key.split("@")[1]);
				word.set(key.split("@")[0] + "=" + count);
				context.write(doc, word);
			}
		}
		
    }

    /*
	 * For each identical key (document), reduces the values (word=wordCount) into a sum (docSize) 
	 *
	 * Input:  ( document , (word=wordCount) )
	 * Output: ( (word@document) , (wordCount/docSize) )
	 *
	 * docSize = total number of words in the document
	 */
	public static class DSReducer extends Reducer<Text, Text, Text, Text> {
		
		public void reduce(Text key, Iterable<Text> valueList, Context context)  throws IOException, InterruptedException{
			int size = 0;
			LinkedList<String> valList = new LinkedList<String>();
			for(Text value : valueList){
				String val = value.toString();
				valList.add(val);
				String num = val.split("=")[1];
				if(num.matches("[0-9]+") && !num.equals("")){
					size += Integer.parseInt(num);
				}
			}
			System.out.println(size);
			for(String val : valList){
				String wc = val.split("=")[1];
				Text newKey = new Text(val.split("=")[0] + "@" + key.toString());
				Text newVal = new Text(wc + "/" + size);
				context.write(newKey, newVal);
			}
		}
    }
	
	/*
	 * Rearranges the (key,value) pairs to have only the word as the key
	 * 
	 * Input:  ( (word@document) , (wordCount/docSize) )
	 * Output: ( word , (document=wordCount/docSize) )
	 */
	public static class TFICFMapper extends Mapper<Object, Text, Text, Text> {

		private Text word = new Text();
		private Text val = new Text();
		/**
		* Map function of mapper parent class that takes aline of text and returns the count of each word
		*/
		@Override
		protected void map(Object info, Text value, Context context)  throws IOException, InterruptedException{
			String information = value.toString();
			if(information.contains("@")){
				StringTokenizer st = new StringTokenizer(information);
				String key = st.nextToken();
				String size = st.nextToken();
				word.set(key.split("@")[0]);
				val.set(key.split("@")[1] + "=" + size);
				context.write(word, val);
			}
			
		}
		
    }

    /*
	 * For each identical key (word), reduces the values (document=wordCount/docSize) into a 
	 * the final TFICF value (TFICF). Along the way, calculates the total number of documents and 
	 * the number of documents that contain the word.
	 * 
	 * Input:  ( word , (document=wordCount/docSize) )
	 * Output: ( (document@word) , TFICF )
	 *
	 * numDocs = total number of documents
	 * numDocsWithWord = number of documents containing word
	 * TFICF = ln(wordCount/docSize + 1) * ln(numDocs/numDocsWithWord +1)
	 *
	 * Note: The output (key,value) pairs are sorted using TreeMap ONLY for grading purposes. For
	 *       extremely large datasets, having a for loop iterate through all the (key,value) pairs 
	 *       is highly inefficient!
	 */
	public static class TFICFReducer extends Reducer<Text, Text, Text, Text> {
		
		private static int numDocs;
		private Map<Text, Text> tficfMap = new HashMap<>();
		
		// gets the numDocs value and stores it
		protected void setup(Context context) throws IOException, InterruptedException {
			Configuration conf = context.getConfiguration();
			numDocs = Integer.parseInt(conf.get("numDocs"));
		}
		
		public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
			double numDocsWithWord = 0;
			LinkedList<String> valList = new LinkedList<String>();
			for(Text value : values){
				numDocsWithWord++;
				String val = value.toString();
				valList.add(val);
			}
			for(String val : valList){
				String doc = val.split("=")[0];
				String fraction = val.split("=")[1];
				double count = Double.parseDouble(fraction.split("/")[0]);
				double size = Double.parseDouble(fraction.split("/")[1]);
				double tficf = Math.log(count / size + 1) * Math.log(numDocs / numDocsWithWord + 1);
				Text newKey = new Text(doc + "@" + key);
				Text newVal = new Text(String.valueOf(tficf));
				tficfMap.put(newKey, newVal);
			}
		}
		
		// sorts the output (key,value) pairs that are contained in the tficfMap
		protected void cleanup(Context context) throws IOException, InterruptedException {
            Map<Text, Text> sortedMap = new TreeMap<Text, Text>(tficfMap);
			for (Text key : sortedMap.keySet()) {
                context.write(key, sortedMap.get(key));
            }
        }
		
    }
}
