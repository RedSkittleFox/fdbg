#include <unordered_map>
#include <optional>

#include <fdbg/win32_helpers/dbg_help.hpp>
#include <fdbg/dbg/process.hpp>

#include <fdbg/controller/c_threads.hpp>

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
	GetThreadContext(dbg_thread(), &context);
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

void* imagehlp_get_line_address(const std::string& name_, size_t line_)
{
	// I want to know the reason why this doesn't work,
	// email me at RedSkittleFox@gmail.com if you know this :P
	// 	   
	// if (line_ == 0) return nullptr;
	// 
	// std::vector<DWORD64> buf(line_, 0x0u);
	// 
	// ULONG rv = ::SymGetFileLineOffsets64(
	// 	dbg_process(),
	// 	nullptr,
	// 	name_.c_str(),
	// 	buf.data(),
	// 	buf.size()
	// );
	// 
	// if (rv == 0) return nullptr;
	// if (rv < line_) return nullptr;
	// 
	// return std::bit_cast<void*>(buf[rv - 1]);

	// Worse but working solution

	// Enumerate modules
	// TODO: Use internal register
	std::vector<DWORD64> modules;
	auto sym_enum_modules_callback = [](const char* name_, DWORD64 base_, void* vec_) -> BOOL CALLBACK
	{
		using vec_t = decltype(modules);
		vec_t* p_mod = std::bit_cast<vec_t*>(vec_);

		p_mod->push_back(base_);
		return true;
	};

	if (SymEnumerateModules64(dbg_process(), sym_enum_modules_callback, &modules) == FALSE) return nullptr;

	// Enumerate source lines
	struct
	{
		void* address;
		size_t line_number;
	} sym_enum_lines_callback_context
	{
		.address = nullptr,
		.line_number = line_
	};

	auto sym_enum_lines_callback = [](PSRCCODEINFO info_, void* user_context_) -> BOOL CALLBACK
	{
		using callback_t = decltype(sym_enum_lines_callback_context);
		callback_t* addr = std::bit_cast<callback_t*>(user_context_);

		// We want to find the next closest line to the statement
		if (info_->LineNumber == addr->line_number)
		{
			addr->address = std::bit_cast<void*>(info_->Address);
			return false;
		}

		return true;
	};

	for (auto mod : modules)
	{
		if (SymEnumLines(dbg_process(), mod, nullptr, name_.c_str(),
			sym_enum_lines_callback, &sym_enum_lines_callback_context) == FALSE) continue;

		if (sym_enum_lines_callback_context.address != nullptr) break;
	}

	return sym_enum_lines_callback_context.address;
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
	// Get Symbol name
	auto name = imagehlp_get_name(id_, base_);

	auto res = m_types.find(name);
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

// This causes stack overflow, idk why, we don't need it anyway
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

	// Get the type of the *type*
	DWORD tag = {};
	BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_SYMTAG, &tag);

	imagehlp_symbol_type* type{ nullptr };

	bool has_size = false;
	switch (tag)
	{
	case (SymTagBaseType):
	{
		// Get fundamental type index
		DWORD fundamental_idx = {};
		res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_BASETYPE, &fundamental_idx);

		// Get size
		ULONG64 size = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_LENGTH, &size);

		std::string name = imagehlp_get_fundamental_name(static_cast<imagehlp_symbol_type_fundamental_type>(fundamental_idx), size);
		
		if (auto res = m_types.find(name); res != std::end(m_types)) 
			return &res->second;
		
		type = &m_types[name];
		type->name = name;
		type->id = id_;
		type->size = size;

		// Get fundamental type type
		imagehlp_symbol_type::fundamental u;
		u.type = static_cast<imagehlp_symbol_type_fundamental_type>(fundamental_idx);
		type->u = u;
		break;
	}
	case (SymTagEnum):
	{
		std::string name = imagehlp_get_name(id_, base_);

		if (auto res = m_types.find(name); res != std::end(m_types))
			return &res->second;

		type = &m_types[name];
		type->name = name;
		type->id = id_;

		imagehlp_symbol_type::enumeration u;
		has_size = true;
		type->u = u;
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

		std::string name = u.type->name + '[' + std::to_string(u.size) + ']';
		if (auto res = m_types.find(name); res != std::end(m_types))
			return &res->second;

		type = &m_types[name];
		type->name = name;
		type->id = id_;

		type->u = u;
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
			std::string name = u.type->name + '*';
			
			if (auto res = m_types.find(name); res != std::end(m_types))
				return &res->second;

			type = &m_types[name];
			type->name = name;
			type->id = id_;

			type->u = u;
		}
		else
		{
			imagehlp_symbol_type::reference u;
			has_size = true;

			// Get array's type
			DWORD type_id = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
			u.type = this->type(type_id, base_);
			std::string name = u.type->name + '&';

			if (auto res = m_types.find(name); res != std::end(m_types))
				return &res->second;

			type = &m_types[name];
			type->name = name;
			type->id = id_;

			type->u = u;
		}
		break;
	}
	case(SymTagTypedef):
	{
		std::string name = imagehlp_get_name(id_, base_);
		if (auto res = m_types.find(name); res != std::end(m_types))
			return &res->second;

		type = &m_types[name];
		type->name = name;
		type->id = id_;

		has_size = false;
		imagehlp_symbol_type::type_def u;

		DWORD type_id = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &type_id);
		u.type = this->type(type_id, base_);

		type->size = u.type->size;
		type->u = u;
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

		std::string name = u.return_type->name;
		
		// (Parent::)
		if (u.parent != nullptr)
		{
			name += '(' + u.parent->name + "::)";
		}

		name += '(';
		for (size_t i = {}; i < u.parameters.size(); ++i)
		{
			auto& parmams = u.parameters[i];
			if (i + 1 != u.parameters.size())
				name += parmams->name + ", ";
		};
		name += ')';

		if (auto res = m_types.find(name); res != std::end(m_types))
			return &res->second;

		type = &m_types[name];
		type->name = name;
		type->id = id_;
		type->u = u;

		break;
	}
	case(SymTagUDT): // User defined type (struct, class, union)
	{
		has_size = true;
		imagehlp_symbol_type::user_defined u;

		std::string name = imagehlp_get_name(id_, base_);
		if (auto res = m_types.find(name); res != std::end(m_types))
			return &res->second;

		type = &m_types[name];
		type->name = name;
		type->id = id_;

		// Get kind
		{
			DWORD kind = {};
			BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_TYPE, &kind);
			// TODO: Is this safe?
			u.kind = static_cast<imagehlp_symbol_type_udt_kind>(kind);
		}

		// Get Children, those can be "variables", "functions", "base classes"
		auto children = imagehlp_get_children(id_, base_);

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

		type->u = u;
		
		break;
	}
	default:
		return nullptr;
	};

	if (has_size && type != nullptr)
	{
		// Get pointer's size
		ULONG64 size = {};
		BOOL res = SymGetTypeInfo(process::instance().handle(), base_, id_, TI_GET_LENGTH, &size);
		type->size = size;
	}
	
	return type;
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

HANDLE dbg_process()
{
	return process::instance().handle();
}

HANDLE dbg_thread()
{
	return mvc<threads_controller>().current_thread().handle;
}