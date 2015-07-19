package org.easyrpg.player.game_browser;

public class ProjectInformation {
	private String title, path;

	public ProjectInformation(String title, String path) {
		super();
		this.title = title;
		this.path = path;
	}

	public String getTitle() {
		return title;
	}
	
	public String getPath() {
		return path;
	}
}