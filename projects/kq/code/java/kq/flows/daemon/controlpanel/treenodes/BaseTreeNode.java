package kq.flows.daemon.controlpanel.treenodes;

import java.util.Enumeration;

import javax.swing.JPanel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeNode;

public class BaseTreeNode implements MutableTreeNode {
	private DefaultMutableTreeNode treeNode;
	static private JPanel emptyPanel;

	/**
	 * Create the panel.
	 */
	public BaseTreeNode(String sRootName) {
		if(emptyPanel == null){
			emptyPanel = new JPanel();
		}
		treeNode = new DefaultMutableTreeNode(sRootName);
	}

	@SuppressWarnings("rawtypes")
	@Override
	public Enumeration children() {
		return treeNode.children();
	}

	@Override
	public boolean getAllowsChildren() {
		return treeNode.getAllowsChildren();
	}

	@Override
	public TreeNode getChildAt(int arg0) {
		return treeNode.getChildAt(arg0);
	}

	@Override
	public int getChildCount() {
		return treeNode.getChildCount();
	}

	@Override
	public int getIndex(TreeNode arg0) {
		return treeNode.getIndex(arg0);
	}

	@Override
	public TreeNode getParent() {
		return treeNode.getParent();
	}

	@Override
	public boolean isLeaf() {
		return treeNode.isLeaf();
	}

	@Override
	public void insert(MutableTreeNode child, int index) {
		treeNode.insert(child, index);
	}

	@Override
	public void remove(int index) {
		treeNode.remove(index);
	}

	@Override
	public void remove(MutableTreeNode node) {
		treeNode.remove(node);
	}

	@Override
	public void removeFromParent() {
		treeNode.removeFromParent();
	}

	@Override
	public void setParent(MutableTreeNode newParent) {
		treeNode.setParent(newParent);
	}

	@Override
	public void setUserObject(Object object) {
		treeNode.setUserObject(object);
	}
	
	public Object getUserObject(){
		return treeNode.getUserObject();
	}
	
	
	public JPanel getOptionsPanel(){
		return emptyPanel;
	}

	public String toString(){
		return getUserObject().toString();
	}
}
