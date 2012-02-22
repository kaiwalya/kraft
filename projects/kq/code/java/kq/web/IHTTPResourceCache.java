package kq.web;

import java.io.*;
import java.net.*;

public interface IHTTPResourceCache{
	public InputStream getResource(URI uri, String sContentType);
}
