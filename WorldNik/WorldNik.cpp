#include "stdafx.h"
#include "Variables.h"
#include "Initialize.h"
#include <osg/Program>
#include <osg/LineWidth>

static const char* vertSource = {
	"#version 120\n"
	"#extension GL_EXT_geometry_shader4 : enable\n"
	"void main()\n"
	"{ gl_Position = ftransform(); }\n"
};

static const char* geomSource = {
	"#version 120\n"
	"#extension GL_EXT_geometry_shader4 : enable\n"
	"uniform int segments;\n"
	"void main(void)\n"
	"{\n"
	"    float delta = 1.0 / float(segments);\n"
	"    vec4 v;\n"
	"    for ( int i=0; i<=segments; ++i )\n"
	"    {\n"
	"        float t = delta * float(i);\n"
	"        float t2 = t * t;\n"
	"        float one_minus_t = 1.0 - t;\n"
	"        float one_minus_t2 = one_minus_t * one_minus_t;\n"
	"        v = gl_PositionIn[0] * one_minus_t2 * one_minus_t +\n"
	"                      gl_PositionIn[1] * 3.0 * t * one_minus_t2 +\n"
	"                      gl_PositionIn[2] * 3.0 * t2 * one_minus_t +\n"
	"                      gl_PositionIn[3] * t2 * t;\n"
	"        gl_Position = v;\n"
	"        EmitVertex();\n"
	"    }\n"
	"    EndPrimitive();\n"
	"}\n"
};

using namespace osgEarth;

int _tmain(int argc, _TCHAR* argv[])
{
	boost::filesystem::path full_path(boost::filesystem::initial_path<boost::filesystem::path>());
	full_path = boost::filesystem::system_complete(boost::filesystem::path(argv[0]));
	PATH = full_path.string().substr(0, full_path.string().size() - 12);

	std::cout << "Your current working path:\n\t---> " << PATH << "\n" << std::endl;

	std::cout << "Please select Data Source:\n" <<
		"0\tDatabase\n" <<
		"1\tFiles\n" <<
		"You have selected: ";
	int choice = 0;
	std::cin >> choice;
	std::cout << std::endl;

	switch (choice)
	{
	case 0:
		dataSource = DB_SOURCE;
		break;
	case 1:
		dataSource = FILE_SOURCE;
		std::cout << "Number of planes to load: ";
		std::cin >> numPlanesToGet;
		std::cout << std::endl;
		break;
	}

	Initialize();

	osgEarth::Viewpoint view(-22.6055, 63.985, 500, -8.51114, -12.9368, 15678.1, geoSRS);
	//osgEarth::Viewpoint view(0, 60, 500, -8.51114, -12.9368, 20000, geoSRS);
	e_manip->setViewpoint(view, 1.0);
	
	/*
	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
	for (int i = 0; i < planePoints[0].size(); i++)
	{
		GeoPoint* p = new GeoPoint(geoSRS, planePoints[0][i].lon, planePoints[0][i].lat, planePoints[0][i].alt, ALTMODE_ABSOLUTE);
		osg::Vec3d asd;
		p->toWorld(asd);
		vertices->push_back(asd);
	}

	osg::ref_ptr<osg::Geometry> controlPoints = new osg::Geometry;
	controlPoints->setVertexArray(vertices.get());
	controlPoints->addPrimitiveSet(new osg::DrawArrays(GL_LINES_ADJACENCY_EXT, 0, planePoints[0].size()));

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	geode->addDrawable(controlPoints.get());

	int segments = planePoints[0].size() * 2 + 2;

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(new osg::Shader(osg::Shader::VERTEX, vertSource));
	program->addShader(new osg::Shader(osg::Shader::GEOMETRY, geomSource));
	program->setParameter(GL_GEOMETRY_VERTICES_OUT_EXT, segments + 1);
	program->setParameter(GL_GEOMETRY_INPUT_TYPE_EXT, GL_LINES_ADJACENCY_EXT);
	program->setParameter(GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_LINE_STRIP);

	osg::ref_ptr<osg::LineWidth> lineWidth = new osg::LineWidth;
	lineWidth->setWidth(20.0f);

	osg::StateSet* stateset = geode->getOrCreateStateSet();
	stateset->setAttributeAndModes(program.get());
	stateset->setAttribute(lineWidth.get());
	stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	stateset->addUniform(new osg::Uniform("segments", segments));

	root->addChild(geode.get());
	*/

	viewer.run();
	return 0;
}