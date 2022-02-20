#include "Main.h"
#include "Tools.h"
#include "cpplinq.hpp"
#include "tinyformat.h"

#include "Logger.hpp"

#include "IGenerator.hpp"

#include "ObjectsStore.hpp"
#include "NamesStore.hpp"
#include "Package.hpp"
#include "NameValidator.hpp"

#include "PrintHelper.hpp"

extern IGenerator* generator;

/// <summary>
/// Dumps the objects and names to files.
/// </summary>
/// <param name="path">The path where to create the dumps.</param>
void Dump(std::string path)
{
	{
		std::ofstream o(path + "/" + "ObjectsDump.txt");
		tfm::format(o, "Address: %P\n\n", ObjectsStore::GetAddress());

		for (auto obj : ObjectsStore())
		{
			tfm::format(o, "[%06i] %-100s 0x%P\n", obj.GetIndex(), obj.GetFullName(), obj.GetAddress());
		}
	} 

	{
		std::ofstream o(path + "/" + "NamesDump.txt");
		tfm::format(o, "Address: %P\n\n", NamesStore::GetAddress());

		for (auto name : NamesStore())
		{
			tfm::format(o, "[%06i] %s\n", name.Index, name.Name);
		}
	}
}

void SaveSDKHeader(std::string path, const std::unordered_map<UEObject, bool>& processedObjects, const std::vector<std::unique_ptr<Package>>& packages)
{
    std::ofstream os(path + "/" +  "SDK.hpp");

    os << "#pragma once\n\n"
        << tfm::format("// %s (%s) SDK by Dyno\n\n", generator->GetGameName(), generator->GetGameVersion());

    //Includes
    os << "#include <set>\n";
    os << "#include <string>\n";
    for (auto&& i : generator->GetIncludes())
    {
        os << "#include " << i << "\n";
    }

    //include the basics
    {
        {
            std::ofstream os2(path + "/SDK" + "/" + tfm::format("%s_Basic.hpp", generator->GetGameNameShort()));
            std::vector<std::string> incs = {
            "<iostream>",
            "<string>",
            "<unordered_set>",
            "<codecvt>"
            };
            PrintFileHeader(os2, incs, true);
            
            os2 << generator->GetBasicDeclarations() << "\n";

            PrintFileFooter(os2);

            os << "\n#include \"SDK/" << tfm::format("%s_Basic.hpp", generator->GetGameNameShort()) << "\"\n";
        }
        {
            std::ofstream os2(path + "/SDK" +  "/" +  tfm::format("%s_Basic.cpp", generator->GetGameNameShort()));

            PrintFileHeader(os2, { "\"../SDK.hpp\"" }, false);

            os2 << generator->GetBasicDefinitions() << "\n";

            PrintFileFooter(os2);
        }
    }

    using namespace cpplinq;

    //check for missing structs
    const auto missing = from(processedObjects) >> where([](auto&& kv) { return kv.second == false; });
    if (missing >> any())
    {
        std::ofstream os2(path + "/SDK" + "/" + tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()));

        PrintFileHeader(os2, true);

       /* 
       for (auto&& s : missing >> select([](auto&& kv) { return kv.first.Cast<UEStruct>(); }) >> experimental::container())
        {
            os2 << "// " << s.GetFullName() << "\n// ";
            os2 << tfm::format("0x%04X\n", s.GetPropertySize());

            os2 << "struct " << MakeValidName(s.GetNameCPP()) << "\n{\n";
            os2 << "\tunsigned char UnknownData[0x" << tfm::format("%X", s.GetPropertySize()) << "];\n};\n\n";
        }
        */

        PrintFileFooter(os2);

        os << "\n#include \"SDK/" << tfm::format("%s_MISSING.hpp", generator->GetGameNameShort()) << "\"\n";
    }

    os << "\n";

    for (auto&& package : packages)
    {
        os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Structs, *package) << "\"\n";
        os << R"(#include "SDK/)" << GenerateFileName(FileContentType::Classes, *package) << "\"\n";
        if (generator->ShouldGenerateFunctionParametersFile())
        {
            os << R"(#include "SDK/)" << GenerateFileName(FileContentType::FunctionParameters, *package) << "\"\n";
        }
    }
}

/// <summary>
/// Process the packages.
/// </summary>
/// <param name="path">The path where to create the package files.</param>
void ProcessPackages(std::string path)
{
    using namespace cpplinq;

    const auto sdkPath = path + "/SDK";
    mkdir(sdkPath.c_str(), 0777);
    
    std::vector<std::unique_ptr<Package>> packages;

    std::unordered_map<UEObject, bool> processedObjects;

    auto packageObjects = from(ObjectsStore())
        >> select([](auto&& o) { return o.GetPackageObject(); })
        >> where([](auto&& o) { return o.IsValid(); })
        >> distinct()
        >> to_vector();

    for (auto obj : packageObjects)
    {
        auto package = std::make_unique<Package>(obj);

        package->Process(processedObjects);
        if (package->Save(sdkPath))
        {
            Package::PackageMap[obj] = package.get();

            packages.emplace_back(std::move(package));
        }
    }

    if (!packages.empty())
    {
        // std::sort doesn't work, so use a simple bubble sort
        //std::sort(std::begin(packages), std::end(packages), PackageDependencyComparer());
        const PackageDependencyComparer comparer;
        for (auto i = 0u; i < packages.size() - 1; ++i)
        {
            for (auto j = 0u; j < packages.size() - i - 1; ++j)
            {
                if (!comparer(packages[j], packages[j + 1]))
                {
                    std::swap(packages[j], packages[j + 1]);
                }
            }
        }
    }

    SaveSDKHeader(path, processedObjects, packages);
}

void *main_thread(void *) {
    
    UE4 = Tools::GetBaseAddress(lib_name);
    while (!UE4) {
        UE4 = Tools::GetBaseAddress(lib_name);
        sleep(1);
    }
    
    //Delay
    sleep(5);
    
    uintptr_t GName_Addr = getTNameArrayAddr();
    uintptr_t GObject_Addr = getFUObjecrtArrayAddr();
        
    if (!ObjectsStore::Initialize(GObject_Addr))
    {
        return ;
    }
    if (!NamesStore::Initialize(GName_Addr))
    {
        return ;
    }

    if (!generator->Initialize(UE4))
    {
        return ;
    }
    
    //file1 << NamesStore().GetById(2).c_str() << endl;
    //file1 << dec <<  ObjectsStore().GetObjectsNum() << endl;

    std::string outputDirectory = generator->GetOutputDirectory(pkgName);
    
    #if defined(__LP64__)
    outputDirectory += "/" + generator->GetGameNameShort() + "_(v" + generator->GetGameVersion() + ")_64Bit";
    #else
    outputDirectory += "/" + generator->GetGameNameShort() + "_(v" + generator->GetGameVersion() + ")_32Bit";
    #endif
    
	mkdir(outputDirectory.c_str(), 0777);
    
    std::ofstream log(outputDirectory + "/Generator.log");
	Logger::SetStream(&log);
    
    Logger::Log("Cheking LOGs");
    
    Dump(outputDirectory);
    
    const auto begin = std::chrono::system_clock::now();

    ProcessPackages(outputDirectory);

    Logger::Log("Finished, took %d seconds.", std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - begin).count());
    
    return 0;
}

extern "C"
JNIEXPORT int JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
     Media_Folder = "/storage/emulated/0/Android/media/" + pkgName;
    
    struct stat info;
    if( stat( Media_Folder.c_str(), &info ) != 0 ){
    LOGE( "cannot access %s\n", Media_Folder.c_str() );
    mkdir(Media_Folder.c_str(), 0777);
    }
    
	// file1.open(Media_Folder + "/gen_log.txt");

   pthread_t m_thread;
   pthread_create(&m_thread, 0, main_thread, 0);
    
   return JNI_VERSION_1_6;
}

extern "C"
{
void __attribute__ ((visibility ("default"))) OnLoad() 
{
    
    Media_Folder = "/storage/emulated/0/Android/media/" + pkgName;
    
    struct stat info;
    if( stat( Media_Folder.c_str(), &info ) != 0 ){
    LOGE( "cannot access %s\n", Media_Folder.c_str() );
    mkdir(Media_Folder.c_str(), 0777);
    }
    
    //file1.open(Media_Folder + "/gen_log.txt");
     
      pthread_t thread;
      pthread_create(&thread, 0, main_thread, 0);
      
}
}
