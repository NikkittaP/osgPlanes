#ifndef NodeVisitors_HEADER
#define NodeVisitors_HEADER

class  CcolorVisitor : public osg::NodeVisitor {
public:
	CcolorVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		// ---------------------------------------------------------------
		//
		// Default Ctors overide the default node visitor mode so all
		// children are visited
		//
		// ---------------------------------------------------------------

		//
		// Default to a white color
		//
		m_color.set(1.0, 1.0, 1.0, 1.0);
		m_colorArrays = new osg::Vec4Array;
		m_colorArrays->push_back(m_color);
	};
	CcolorVisitor(const osg::Vec4 &color) : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		// -------------------------------------------------------------------
		//
		// Overloaded Ctor initialised with the Color
		// Also override the visitor to traverse all the nodes children
		//
		// -------------------------------------------------------------------
		m_color = m_color;
		m_colorArrays = new osg::Vec4Array;
		m_colorArrays->push_back(m_color);
	};
	virtual ~CcolorVisitor(){};
	virtual
		void apply(osg::Node &node){
			// --------------------------------------------
			//
			//  Handle traversal of osg::Node node types
			//
			// --------------------------------------------
			traverse(node);
		} // apply( osg::Node &node )
	virtual
		void apply(osg::Geode &geode){
			// ------------------------------------------------
			//
			//  Handle traversal of osg::Geode node types
			//
			// ------------------------------------------------
			osg::StateSet *state = NULL;
			unsigned int    vertNum = 0;

			//  
			//  We need to iterate through all the drawables check if
			//  the contain any geometry that we will need to process
			//
			unsigned int numGeoms = geode.getNumDrawables();
			for (unsigned int geodeIdx = 0; geodeIdx < numGeoms; geodeIdx++) {

				//
				// Use 'asGeometry' as its supposed to be faster than a dynamic_cast
				// every little saving counts
				//
				osg::Geometry *curGeom = geode.getDrawable(geodeIdx)->asGeometry();
				//
				// Only process if the drawable is geometry
				//
				if (curGeom) {
					osg::Vec4Array *colorArrays = dynamic_cast<osg::Vec4Array *>(curGeom->getColorArray());
					if (colorArrays) {
						for (unsigned int i = 0; i < colorArrays->size(); i++) {
							osg::Vec4 *color = &colorArrays->operator [](i);
							//
							// could also use *color = m_color
							//
							color->set(m_color._v[0], m_color._v[1], m_color._v[2], m_color._v[3]);
						}
					}
					else{
						curGeom->setColorArray(m_colorArrays.get());
						curGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
					}
				}
			}
		} // apply( osg::Geode    
	void
		setColor(const float r, const float g, const float b, const float a = 1.0f){
			// -------------------------------------------------------------------
			//
			// Set the color to change apply to the nodes geometry
			//
			// -------------------------------------------------------------------
			osg::Vec4 *c = &m_colorArrays->operator [](0);
			m_color.set(r, g, b, a);
			*c = m_color;
		} // setColor( r,g,b,a )
	void
		setColor(const osg::Vec4 &color){
			// -------------------------------------------------------------------
			//
			// Set the color to change apply to the nodes geometry
			//
			// -------------------------------------------------------------------
			osg::Vec4 *c = &m_colorArrays->operator [](0);
			m_color = color;
			*c = m_color;
		} // setColor( vec4 )
private:
	osg::Vec4 m_color;
	osg::ref_ptr< osg::Vec4Array > m_colorArrays;
};
void InitColors();

class FindNodeVisitor : public osg::NodeVisitor
{
public:

	// Default constructor - initialize searchForName to "" and 
	// set the traversal mode to TRAVERSE_ALL_CHILDREN
	FindNodeVisitor();

	// Constructor that accepts string argument
	// Initializes searchForName to user string
	// set the traversal mode to TRAVERSE_ALL_CHILDREN
	FindNodeVisitor(const std::string &searchName);

	// The 'apply' method for 'node' type instances.
	// Compare the 'searchForName' data member against the node's name.
	// If the strings match, add this node to our list
	virtual void apply(osg::Node &searchNode);

	// Set the searchForName to user-defined string
	void setNameToFind(const std::string &searchName);

	// Return a pointer to the first node in the list
	// with a matching name
	osg::Node* getFirst();

	// typedef a vector of node pointers for convenience
	typedef std::vector<osg::Node*> nodeListType;

	// return a reference to the list of nodes we found
	nodeListType& getNodeList() { return foundNodeList; }

private:

	// the name we are looking for
	std::string searchForName;

	// List of nodes with names that match the searchForName string
	nodeListType foundNodeList;

};

#endif