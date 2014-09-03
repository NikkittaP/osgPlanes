#include "stdafx.h"
#include "Variables.h"
#include "NodeVisitors.h"

CcolorVisitor blueColor, redColor, whiteColor;
void InitColors()
{
	whiteColor.setColor(osg::Vec4(1, 1, 1, 1));
	blueColor.setColor(osg::Vec4(0, 0, 1, 1));
	redColor.setColor(osg::Vec4(1, 0, 0, 1));
}

FindNamedNode::FindNamedNode(const std::string& name) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _name(name) {}

void FindNamedNode::apply(osg::Node& node)
{
	if (node.getName() == _name)
		_node = &node;
	else
		traverse(node);
}

osg::Node* FindNamedNode::getNode() { return _node.get(); }