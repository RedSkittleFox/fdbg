#include <unordered_map>
#include <optional>

#include <fdbg/dbg/threads.hpp>
#include <fdbg/win32_helpers/dbg_help.hpp>
#include <fdbg/dbg/process.hpp>

///////////////////////////////////////////////////////////////////////////////
// dbg_help and DIA structs
//

// Source:
// https://docs.microsoft.com/pl-pl/visualstudio/debugger/debug-interface-access/symtagenum?view=vs-2019
enum SymTagEnum : DWORD
{
	SymTagNull,
	SymTagExe,
	SymTagCompiland,
	SymTagCompilandDetails,
	SymTagCompilandEnv,
	SymTagFunction,
	SymTagBlock,
	SymTagData,
	SymTagAnnotation,
	SymTagLabel,
	SymTagPublicSymbol,
	SymTagUDT,
	SymTagEnum,
	SymTagFunctionType,
	SymTagPointerType,
	SymTagArrayType,
	SymTagBaseType,
	SymTagTypedef,
	SymTagBaseClass,
	SymTagFriend,
	SymTagFunctionArgType,
	SymTagFuncDebugStart,
	SymTagFuncDebugEnd,
	SymTagUsingNamespace,
	SymTagVTableShape,
	SymTagVTable,
	SymTagCustom,
	SymTagThunk,
	SymTagCustomType,
	SymTagManagedType,
	SymTagDimension,
	SymTagCallSite,
	SymTagInlineSite,
	SymTagBaseInterface,
	SymTagVectorType,
	SymTagMatrixType,
	SymTagHLSLType
};

// Source:
// https://docs.microsoft.com/pl-pl/visualstudio/debugger/debug-interface-access/udtkind?view=vs-2019
enum class UdtKind : DWORD
{
	UdtStruct,
	UdtClass,
	UdtUnion,
	UdtInterface
};

// Source:
// https://docs.microsoft.com/pl-pl/visualstudio/debugger/debug-interface-access/datakind?view=vs-2019
enum class DataKind : DWORD
{
	DataIsUnknown,
	DataIsLocal,
	DataIsStaticLocal,
	DataIsParam,
	DataIsObjectPtr,
	DataIsFileStatic,
	DataIsGlobal,
	DataIsMember,
	DataIsStaticMember,
	DataIsConstant
};

// Source:
// https://docs.microsoft.com/pl-pl/visualstudio/debugger/debug-interface-access/locationtype?view=vs-2019
enum class LocationType : DWORD
{
	LocIsNull,
	LocIsStatic,
	LocIsTLS,
	LocIsRegRel,
	LocIsThisRel,
	LocIsEnregistered,
	LocIsBitField,
	LocIsSlot,
	LocIsIlRel,
	LocInMetaData,
	LocIsConstant,
	LocTypeMax
};

CONTEXT current_context()
{
	CONTEXT context = {};
	context.ContextFlags = CONTEXT_FULL;
	GetThreadContext(threads::instance().current_thread().handle, &context);
	return context;
}

///////////////////////////////////////////////////////////////////////////////
// functions
//

std::vector<DWORD> imagehlp_get_children(DWORD id_, DWORD64 base_)
{
	// Get children count
	DWORD children_count = {};
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_CHILDRENCOUNT, &children_count);
	if (children_count == 0 || res == false)
		return std::vector<DWORD>();

	// Get children
	std::vector<std::byte> children_params(sizeof(TI_FINDCHILDREN_PARAMS) + children_count * sizeof(DWORD), std::byte(0));
	std::bit_cast<TI_FINDCHILDREN_PARAMS*>(children_params.data())->Count = children_count;
	std::bit_cast<TI_FINDCHILDREN_PARAMS*>(children_params.data())->Start = 0;
	res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_FINDCHILDREN, children_params.data());

	if (res == false)
		return std::vector<DWORD>();

	std::vector<DWORD> ret;
	ret.reserve(children_count);
	
	for (size_t i = {}; i < children_count; ++i)
	{
		ret.push_back(std::bit_cast<TI_FINDCHILDREN_PARAMS*>(children_params.data())->ChildId[i]);
	}

	return ret;
}

std::string imagehlp_get_name(DWORD id_, DWORD64 base_)
{
	std::string ret;
	wchar_t* name = nullptr;
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_SYMNAME, &name);
	if (res != false && name != nullptr)
	{
		std::wstring wname(name);
		ret = to_string_from_wstring(wname);
		// SymGetTypeInfo requires us to free the memory. 
		// Refer to the documentation for more details.
		LocalFree(name);
	}

	return ret;
}

std::string imagehlp_get_fundamental_name(imagehlp_symbol_type_fundamental_type type_, size_t size_)
{
	using e = imagehlp_symbol_type_fundamental_type;
	std::string name;
	switch (type_)
	{
	case e::VOID_T:
		name = "void";
		break;
	case e::CHAR:
		name = "char";
		break;
	case e::WCHAR:
		name = "wchar_t";
		break;
	case e::INT:
	{
		if (size_ == sizeof(int))
			name = "int";
		else if (size_ == sizeof(short int))
			name = "short int";
		else
			name = "int";
		break;
	}
	case e::UNSIGNED_INT:
	{
		if (size_ == sizeof(unsigned int))
			name = "unsigned int";
		else if (size_ == sizeof(unsigned short int))
			name = "unsigned short int";
		else
			name = "unsigned int";
		break;
	}
	case e::FLOAT:
	{
		if (size_ == sizeof(float))
			name = "float";
		else if (size_ == sizeof(double))
			name = "double";
		else if (size_ == sizeof(long double))
			name = "long double";
		else
			name = "float";
		break;
	}
	case e::BOOL:
		name = "bool";
		break;
	case e::LONG:
		name = "long";
		break;
	case e::UNSIGNED_LONG:
		name = "unsigned long";
		break;
	case e::HRESULT:    // HRESULT
		name = "HRESULT";
		break;
	case e::CHAR16_T:
		name = "char16_t";
		break;
	case e::CHAR32_T:
		name = "char32_t";
		break;
	default:
		name = "<unknown>";
		break;
	}
	return name;
}

///////////////////////////////////////////////////////////////////////////////
// imagehlp_symbol_manager definitions
//

imagehlp_symbol_manager& imagehlp_symbol_manager::instance()
{
	static imagehlp_symbol_manager inst;
	return inst;
}

imagehlp_symbol_type* imagehlp_symbol_manager::type(DWORD id_, DWORD64 base_)
{
	auto res = m_types.find(id_);
	if (res == std::end(m_types))
		return register_type(id_, base_);

	return &res->second;
}

imagehlp_symbol_variable* imagehlp_symbol_manager::varaible(DWORD id_, DWORD64 base_)
{
	auto res = m_variables.find(id_);
	if (res == std::end(m_variables))
		return register_variable(id_, base_);

	return &res->second;
}

imagehlp_symbol_function* imagehlp_symbol_manager::function(DWORD id_, DWORD64 base_)
{
	auto res = m_functions.find(id_);
	if (res == std::end(m_functions))
		return register_function(id_, base_);

	return &res->second;
}

imagehlp_symbol_type* imagehlp_symbol_manager::register_type(DWORD id_, DWORD64 base_)
{
	if (id_ == 0) return nullptr;

	imagehlp_symbol_type type;
	type.id = id_;

	// Get the type of the *type*
	DWORD tag = {};
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_SYMTAG, &tag);

	bool has_size = false;
	switch (tag)
	{
	case (SymTagBaseType):
	{
		imagehlp_symbol_type::fundamental u;

		// Get pointer's size
		ULONG64 size = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_LENGTH, &size);
		type.size = size;

		DWORD fundamental_idx = {};
		res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_BASETYPE, &fundamental_idx);

		// type.name = imagehlp_get_name(id_, base_);
		type.name = imagehlp_get_fundamental_name(static_cast<imagehlp_symbol_type_fundamental_type>(fundamental_idx), size);
		
		// Get fundamental type type
		u.type = static_cast<imagehlp_symbol_type_fundamental_type>(fundamental_idx);
		type.u = u;
		break;
	}
	case (SymTagEnum):
	{
		type.name = imagehlp_get_name(id_, base_);
		imagehlp_symbol_type::enumeration u;
		has_size = true;
		type.u = u;
		break;
	}
	case (SymTagArrayType):
	{

		imagehlp_symbol_type::array u;
		has_size = true;

		// Get array's size
		DWORD count = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_COUNT, &count);
		u.size = count;

		// Get array's type
		DWORD type_id = {};
		res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
		u.type = this->type(type_id, base_);

		type.name = u.type->name + '[' + std::to_string(u.size) + ']';
		type.u = u;
		break;
	}
	case(SymTagPointerType):
	{
		// We can be either a pointer or a reference, check that!
		BOOL reference = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_IS_REFERENCE, &reference);

		if (reference == false)
		{
			imagehlp_symbol_type::pointer u;
			has_size = true;

			// Get array's type
			DWORD type_id = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
			u.type = this->type(type_id, base_);
			type.u = u;
			type.name = u.type->name + '*';
		}
		else
		{
			imagehlp_symbol_type::reference u;
			has_size = true;

			// Get array's type
			DWORD type_id = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
			u.type = this->type(type_id, base_);
			type.u = u;
			type.name = u.type->name + '&';
		}
		break;
	}
	case(SymTagTypedef):
	{
		has_size = false;
		imagehlp_symbol_type::type_def u;

		DWORD type_id = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
		u.type = this->type(type_id, base_);

		type.size = u.type->size;
		type.u = u;
		type.name = imagehlp_get_name(id_, base_);
		break;
	}
	case(SymTagFunctionType):
	{
		has_size = false;
		imagehlp_symbol_type::function u;

		// Get return type
		{
			DWORD type_id = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);

			u.return_type = this->register_type(type_id, base_);
		}

		// Arguments
		{
			auto param_ids = imagehlp_get_children(id_, base_);
			u.parameters.reserve(param_ids.size());
			for (auto p : param_ids)
			{
				u.parameters.push_back(this->varaible(p, base_));
			}
		}

		// Parent
		{
			DWORD parent = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_CLASSPARENTID, &parent);
			u.parent = this->type(parent, base_);
		}

		type.u = u;

		type.name = u.return_type->name;
		
		// (Parent::)
		if (u.parent != nullptr)
		{
			type.name += '(' + u.parent->name + "::)";
		}

		type.name += '(';
		for (size_t i = {}; i < u.parameters.size(); ++i)
		{
			auto& parmams = u.parameters[i];
			if (i + 1 != u.parameters.size())
				type.name += parmams->name + ", ";
		};
		type.name += ')';

		break;
	}
	case(SymTagUDT): // User defined type (struct, class, union)
	{
		has_size = true;
		imagehlp_symbol_type::user_defined u;

		// Get kind
		{
			DWORD kind = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &kind);
			// TODO: Is this safe?
			u.kind = static_cast<imagehlp_symbol_type_udt_kind>(kind);
		}

		// Get Children, those can be "variables", "functions", "base classes"
		auto children = imagehlp_get_children(id_, base_);

		// Get functions
		for (auto id : children)
		{
			imagehlp_symbol_function* function = this->function(id, base_);
			if (function != nullptr)
				u.functions.push_back(function);
		}

		// Get variables
		for (auto id : children)
		{
			imagehlp_symbol_variable* variable = this->varaible(id, base_);
			if (variable != nullptr)
				u.variables.push_back(variable);
		}

		// Get bases
		for (auto id : children)
		{
			imagehlp_symbol_type* base_type = this->type(id, base_);
			if (base_type != nullptr)
				u.base_classes.push_back(base_type);
		}

		type.u = u;
		type.name = imagehlp_get_name(id_, base_);

		break;
	}
	default:
		return nullptr;
	};

	if (has_size)
	{
		// Get pointer's size
		ULONG64 size = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_LENGTH, &size);
		type.size = size;
	}
	
	return &m_types.insert(std::make_pair(id_, type)).first->second;
}

// TODO: TI_GET_ADDRESS
imagehlp_symbol_variable* imagehlp_symbol_manager::register_variable(DWORD id_, DWORD64 base_)
{
	if (id_ == 0) return nullptr;

	// Get the type of the *type*
	DWORD tag = {};
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_SYMTAG, &tag);

	if (tag != SymTagData && tag != SymTagFunctionArgType)
		return nullptr;

	imagehlp_symbol_variable var;
	var.name = imagehlp_get_name(id_, base_);
	
	// Type
	{
		DWORD type = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type);
		var.type = this->type(type, base_);
	}

	// data kind
	{
		DWORD kind = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_DATAKIND, &kind);
		var.data_kind = static_cast<imagehlp_symbol_data_kind>(kind);
	}

	// if we are a member, get the offset
	if (var.data_kind == imagehlp_symbol_data_kind::MEMBER)
	{
		DWORD offset = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_OFFSET, &offset);
		var.address = offset;
	}

	return &(m_variables.insert(std::make_pair(id_, var)).first->second);
}

// TODO: TI_GET_ADDRESS
imagehlp_symbol_function* imagehlp_symbol_manager::register_function(DWORD id_, DWORD64 base_)
{
	if (id_ == 0) return nullptr;

	// Get the type of the *type*
	DWORD tag = {};
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_SYMTAG, &tag);

	if (tag != SymTagFunction)
		return nullptr;

	imagehlp_symbol_function func;
	func.name = imagehlp_get_name(id_, base_);

	// Type
	{
		DWORD type = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type);
		func.type = this->type(type, base_);
	}

	return &(m_functions.insert(std::make_pair(id_, func)).first->second);
}