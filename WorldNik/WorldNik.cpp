#include "stdafx.h"
#include "Variables.h"
#include "Initialize.h"

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

	viewer.run();
	return 0;
}