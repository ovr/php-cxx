#include "function.h"
#include "function_p.h"
#include "value.h"

phpcxx::Function::Function(const char* name, InternalFunction c, const Arguments& required, const Arguments& optional, bool byRef)
    : d_ptr(std::make_shared<FunctionPrivate>(name, c, required, optional, byRef))
{
}

phpcxx::Function::~Function()
{
}

phpcxx::Function& phpcxx::Function::addRequiredArgument(const Argument& arg)
{
    auto& nreq = this->d_ptr->m_fe.num_args;
    auto& args = this->d_ptr->m_arginfo;
    args.insert(args.begin() + nreq + 1, arg.get());
    ++nreq;
    this->d_ptr->m_fe.arg_info = args.data();
    return *this;
}

phpcxx::Function& phpcxx::Function::addOptionalArgument(const Argument& arg)
{
    this->d_ptr->m_arginfo.push_back(arg.get());
    this->d_ptr->m_fe.arg_info = this->d_ptr->m_arginfo.data();
    return *this;
}

phpcxx::Function& phpcxx::Function::setReturnByReference(bool byref)
{
    this->d_ptr->m_arginfo[0].pass_by_reference = byref;
    return *this;
}

phpcxx::Function& phpcxx::Function::setAllowNull(bool allow)
{
    this->d_ptr->m_arginfo[0].allow_null = allow;
    return *this;
}

phpcxx::Function& phpcxx::Function::setTypeHint(ArgumentType t)
{
    this->d_ptr->m_arginfo[0].type_hint = static_cast<zend_uchar>(t);
    return *this;
}

phpcxx::Function& phpcxx::Function::setTypeHint(const char* className)
{
    zend_internal_arg_info& info = this->d_ptr->m_arginfo[0];
    info.type_hint  = IS_OBJECT;
    info.class_name = className;
    return *this;
}

const struct _zend_function_entry& phpcxx::Function::getFE() const
{
    return this->d_ptr->m_fe;
}

const std::vector<struct _zend_internal_arg_info>& phpcxx::Function::getArgInfo() const
{
    return this->d_ptr->m_arginfo;
}