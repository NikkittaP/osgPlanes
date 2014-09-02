#ifndef EventHandlers_HEADER
#define EventHandlers_HEADER

class PickHandler : public osgGA::GUIEventHandler
{
public:
	PickHandler() {}
	bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);
};

struct SpeedHandler : public osgEarth::Util::Controls::ControlEventHandler
{
	SpeedHandler(){}
	void onValueChanged(osgEarth::Util::Controls::Control* control, float value);
	void onClick(osgEarth::Util::Controls::Control* control);
};

struct Toggle : public osgEarth::Util::Controls::ControlEventHandler {
	std::string _type;
	Toggle(std::string type) : _type(type) { }
	void onValueChanged(osgEarth::Util::Controls::Control*, bool value);
};

#endif