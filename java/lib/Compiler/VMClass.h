//===-- VMClass.h - Compiler representation of a Java class -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file was developed by the LLVM research group and is distributed under
// the University of Illinois Open Source License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the VMClass class that represents a
// compile time representation of a Java class (java.lang.Class).
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_JAVA_VMCLASS_H
#define LLVM_JAVA_VMCLASS_H

#include <llvm/Constant.h>
#include <llvm/Module.h>
#include <llvm/Type.h>
#include <map>
#include <string>
#include <vector>

namespace llvm { namespace Java {

  class ClassFile;
  class Resolver;

  class VMClass {
    static const unsigned INVALID_INTERFACE_INDEX = 0xFFFFFFFF;

    const std::string name_;
    Resolver* resolver_;
    const ClassFile* classFile_;
    const VMClass* componentClass_;
    Type* structType_;
    const Type* type_;
    unsigned interfaceIndex_;
    typedef std::map<std::string, int> Field2IndexMap;
    Field2IndexMap f2iMap_;
    typedef std::vector<const Type*> ElementTypes;
    ElementTypes elementTypes_;
    mutable std::vector<void*> resolvedConstantPool_;
    std::vector<const VMClass*> superClasses_;
    std::vector<const VMClass*> interfaces_;

    void addField(const std::string& name, const Type* type);
    void resolveType();

    friend class Resolver;

    // Resolver interface.

    // Load primitive class for type.
    VMClass(Resolver* resolver, const Type* type);

    // Load class by name.
    VMClass(Resolver* resolver, const std::string& className);

    // Load array class of component the passed class.
    VMClass(Resolver* resolver, const VMClass* componentClass);

    // Link the class.
    void link();
    // Resolve the class.
    void resolve();
    // Initialize the class.
    void initialize();

  public:
    const std::string& getName() const { return name_; }
    const Type* getLayoutType() const { return structType_; }
    const Type* getType() const { return type_; }
    const ClassFile* getClassFile() const { return classFile_; }
    unsigned getNumSuperClasses() const { return superClasses_.size(); }
    const VMClass* getSuperClass(unsigned i) const { return superClasses_[i]; }
    const VMClass* getSuperClass() const {
      return getNumSuperClasses() ? getSuperClass(0) : NULL;
    }
    unsigned getNumInterfaces() const { return interfaces_.size(); }
    const VMClass* getInterface(unsigned i) const { return interfaces_[i]; }
    const VMClass* getComponentClass() const { return componentClass_; }
    bool isArray() const { return componentClass_; }
    bool isPrimitive() const { return !structType_; }
    bool isInterface() const { return classFile_ && !getSuperClass(); }
    unsigned getInterfaceIndex() const { return interfaceIndex_; }
    int getFieldIndex(const std::string& name) const;

    llvm::Constant* getConstant(unsigned index) const;
    const VMClass* getClassForClass(unsigned index) const;
    const VMClass* getClassForDescriptor(unsigned index) const;
  };

} } // namespace llvm::Java

#endif//LLVM_JAVA_VMCLASS_H
