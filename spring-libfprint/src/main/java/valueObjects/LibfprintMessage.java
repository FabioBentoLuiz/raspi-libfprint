package valueObjects;

public class LibfprintMessage {

	private String parameter;

	public LibfprintMessage() {

	}
	
	public LibfprintMessage(String parameter) {
		this.parameter = parameter;
	}
	
	public String getParameter() {
		return parameter;
	}

	public void setParameter(String parameter) {
		this.parameter = parameter;
	}

}
