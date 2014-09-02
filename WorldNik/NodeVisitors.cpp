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

FindNodeVisitor::FindNodeVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), searchForName()
{
}
FindNodeVisitor::FindNodeVisitor(const std::string &searchName) : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), searchForName(searchName)
{
}
void FindNodeVisitor::setNameToFind(const std::string &searchName)
{
	searchForName = searchName;
	foundNodeList.clear();
}
osg::Node* FindNodeVisitor::getFirst()
{
	return *(foundNodeList.begin());
}
void FindNodeVisitor::apply(osg::Node &searchNode)
{
	if (searchNode.getName() == searchForName)
	{
		//    std::cout << searchNode.className() << " Name: " << searchNode.getName() << " Lib: " << searchNode.libraryName() << std::endl;
		foundNodeList.push_back(&searchNode);
	}
	traverse(searchNode);
}