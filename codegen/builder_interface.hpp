#ifndef BUILDER_INTERFACE_HPP
#define BUILDER_INTERFACE_HPP

#include "ast.h"
#include "builder_interface.hpp"
#include "declarationnode.h"
#include "systemconfiguration.hpp"

#define STRIDE_PLUGIN_MAX_STR_LEN 32

class Builder;

typedef Builder *(*create_object_t)(std::string projectDir,
                                    std::string strideRoot,
                                    std::string platformPath);
typedef void (*platform_name_t)(char *name);
typedef int (*platform_version_major_t)();
typedef int (*platform_version_minor_t)();

typedef struct {
  create_object_t create;
  platform_name_t get_name;
  platform_version_major_t get_version_major;
  platform_version_minor_t get_version_minor;
} PluginInterface;

class StrideSystem;


class BuilderInterface {
public:
  BuilderInterface(std::string projectDir, std::string strideRoot,
          std::string platformPath)
      : m_projectDir(projectDir), m_strideRoot(strideRoot),
        m_platformPath(platformPath) {}
  virtual ~Builder() {}
  
  std::string getPlatformPath() { return m_platformPath; }
  virtual std::string getStdErr() const = 0;
  virtual std::string getStdOut() const = 0;
  virtual void clearBuffers() = 0;
  
  void registerYieldCallback(std::function<void()> cb) { m_yieldCallback = cb; }
  
  // Additional information from stride system
  std::shared_ptr<StrideSystem>
      m_system; // The StrideSystem that created this builder.
  SystemConfiguration m_systemConfiguration;
  std::string m_frameworkName;
  
public slots:
  virtual std::map<std::string, std::string> generateCode(ASTNode tree) = 0;
  virtual bool build(std::map<std::string, std::string> domainMap) = 0;
  virtual bool deploy() = 0;
  virtual bool run(bool pressed = true) = 0;
  //    // TODO this would need to send encrypted strings or remove the code
  //    sections? virtual QString requestTypesJson() {return "";} virtual
  //    QString requestFunctionsJson() {return "";} virtual QString
  //    requestObjectsJson() {return "";}
  virtual bool isValid() { return false; }
  
protected:
  std::string m_projectDir;
  std::string m_strideRoot;
  std::string m_platformPath;
  
  std::function<void()> m_yieldCallback = []() {};
  
  std::string substituteTokens(std::string text);
  
private:
  PluginInterface m_interface;
};



#endif // BUILDER_INTERFACE_HPP
