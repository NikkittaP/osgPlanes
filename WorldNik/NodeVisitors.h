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

class FindNamedNode : public osg::NodeVisitor
{
public:
	FindNamedNode(const std::string& name);

	// This method gets called for every node in the scene
	//   graph. Check each node to see if its name matches
	//   out target. If so, save the node's address.
	virtual void apply(osg::Node& node);

	osg::Node* getNode();

protected:
	std::string _name;
	osg::ref_ptr<osg::Node> _node;
};

#endif