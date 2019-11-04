package com.demo.libfprint.configurations;

import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties("libfprint")
public class LibfprintProperties {

	private final Uris uris = new Uris();
	
	public Uris getUris() {
		return this.uris;
	}
	
	public static class Uris {
		
		private String enroll;
		private String verify;
		private String identify;
		
		public String getEnroll() {
			return enroll;
		}
		public void setEnroll(String enroll) {
			this.enroll = enroll;
		}
		public String getVerify() {
			return verify;
		}
		public void setVerify(String verify) {
			this.verify = verify;
		}
		public String getIdentify() {
			return identify;
		}
		public void setIdentify(String identify) {
			this.identify = identify;
		}
	}
}

