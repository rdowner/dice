/*
 * DC1/DLANG.C	- D Language prototyping support
 */

#include "defs.h"

Prototype int DumpDProtos(Var *var, Type *type, const char *name, int flags);
Prototype void DumpDStructs(Type *type, const char *name, int flags);

int
DumpDProtos(Var *var, Type *type, const char *name, int flags)
{
    int i;
    int error = 0;
    int32_t off = ftell(stdout);

    if (flags & SSCAN_TOP) {
	if (flags & SSCAN_ERROR)
	    printf("# ");
	printf("public __clang ");
    }
    if (flags & SSCAN_ALIGN)
	printf("__align(%d) ", *type->Align);
    switch(type->Id) {
    case TID_INT:
	if (type->Flags & TF_UNSIGNED) {
	    switch(*type->Size) {
	    case 0:
		printf("void");
		break;
	    case 1:
		printf("int8");
		break;
	    case 2:
		printf("int16");
		break;
	    case 4:
		printf("int32");
		break;
	    case 8:
		printf("int64");
		break;
	    default:
		printf("UNKNOWN_INT_TYPE");
		++error;
		break;
	    }
	} else {
	    switch(*type->Size) {
	    case 0:
		printf("void");
		break;
	    case 1:
		printf("int8");
		break;
	    case 2:
		printf("int16");
		break;
	    case 4:
		printf("int32");
		break;
	    case 8:
		printf("int64");
		break;
	    default:
		printf("UNKNOWN_UINT_TYPE");
		++error;
		break;
	    }
	}
	if (flags & SSCAN_NAME)
	    printf(" %s", name);
	break;
    case TID_FLT:
	switch(*type->Size) {
	case 4:
	    printf("float32");
	    break;
	case 8:
	    printf("float64");
	    break;
	case 16:
	    printf("float128");
	    break;
	default:
	    printf("UNKNOWN_FLOAT_TYPE");
	    ++error;
	    break;
	}
	if (flags & SSCAN_NAME)
	    printf(" %s", name);
	break;
    case TID_ARY:
	if ((flags & SSCAN_ARG) == 0) {
	    error += DumpDProtos(var, type->SubType, name, SSCAN_NAME);
	    if (*type->Size) {
		printf("[%d]", *type->Size / *type->SubType->Size);
	    } else {
		printf("[?]");	/* XXX unknown */
		++error;
	    }
	    break;
	}
	/* fall through to pointer spec if procedure argument or return */
    case TID_PTR:
	if (type->SubType->Id == TID_PROC)
	    ++error;
	error += DumpDProtos(var, type->SubType, NULL, 0);
	if (flags & SSCAN_NAME)
	    printf(" *%s", name);
	else
	    printf(" *");
	break;
    case TID_PROC:
	error += DumpDProtos(var, type->SubType, NULL, SSCAN_ARG); /* return type */
	if (flags & SSCAN_NAME)
	    printf(" %s", name);
	printf("(");
	for (i = 0; i < type->Args; ++i) {
	    Var *arg = type->Vars[i];
	    char *argName;
	    int argFlags = SSCAN_NAME|SSCAN_ARG;

	    if (i)
		printf(", ");
	    if (arg->Sym != NULL) {
		asprintf(&argName, "%*.*s", 
		    arg->Sym->Len, arg->Sym->Len, arg->Sym->Name);
	    } else {
		asprintf(&argName, "arg%d", i + 1);
	    }
#if 0
	    /* stack offsets are not stored here XXX */
	    if ((*arg->Type->Size - 1) & arg->var_Stor.st_Offset)
		argFlags |= SSCAN_ALIGN;
#endif
	    /*
	     * assume possible alignment problem between C and D if the
	     * size is greater then 4.
	     */
	    if (*arg->Type->Size > 4)
		argFlags |= SSCAN_ALIGN;
	    error += DumpDProtos(arg, arg->Type, argName, argFlags);
	    free(argName);
	}
	if (i == 0)
	    printf("void");
	printf(")");
	break;
    case TID_STRUCT:
    case TID_UNION:
	{
	    Symbol *sym;

	    while (type->BasedOnType)
		type = type->BasedOnType;
	    if ((sym = FindStructUnionTag(type)) != NULL) {
		printf("C_%*.*s", sym->Len, sym->Len, sym->Name);
	    } else {
		printf("C_UNKNOWN_STRUCT");
		++error;
	    }
	}
	if (flags & SSCAN_NAME)
	    printf(" %s", name);
	break;
    default:
	++error;
	if (flags & SSCAN_TOP)
	    printf("# Unable to convert %s", name);
	else if (flags & SSCAN_NAME)
	    printf("UNKNOWN_TYPE %s", name);
	else
	    printf("UNKNOWN_TYPE");
    }
    if (flags & SSCAN_TOP) {
	if ((flags & SSCAN_ERROR) == 0 && error) {
	    fseek(stdout, off, 0);
	    DumpDProtos(var, type, name, flags|SSCAN_ERROR);
	} else {
	    printf(";\n");
	}
    }
    return(error);
}

void
DumpDStructs(Type *type, const char *name, int flags)
{
    int i;
    int error = 0;
    int biggest;
    int32_t off;

    if (type->Id != TID_UNION && type->Id != TID_STRUCT)
	return;

    off = ftell(stdout);
    if (type->Id == TID_UNION) {
	printf("# UNION C_%s NOT IMPLEMENTED\n", name);
	return;
    }
    printf("\n");
#if 0
    if (flags & SSCAN_ERROR)
	printf("# ");
#endif
    printf("public __clang ");
    for (i = biggest = 0; i < type->Args; ++i) {
	if (biggest < *type->Vars[i]->Type->Size)
	    biggest = *type->Vars[i]->Type->Size;
    }
    /*
     * force-align under D if we are not sure
     */
    if (biggest > *type->Align)
	printf("__align(%d) ", *type->Align);
    printf("class ");
    printf("C_%s {\n", name);
    for (i = 0; i < type->Args; ++i) {
	Var *arg = type->Vars[i];
	char *argName;
	int argFlags = SSCAN_NAME|SSCAN_TOP;

	/*
	 * force-align under D if we are not sure
	 */
	if ((*arg->Type->Size - 1) & arg->var_Stor.st_Offset)
	    argFlags |= SSCAN_ALIGN;
	if (arg->Sym != NULL) {
	    asprintf(&argName, "%*.*s", 
		arg->Sym->Len, arg->Sym->Len, arg->Sym->Name);
	} else {
	    asprintf(&argName, "arg%d", i + 1);
	}
	if (flags & SSCAN_ERROR)
	    printf("#");
	printf("    ");
	error += DumpDProtos(arg, arg->Type, argName, argFlags);
	free(argName);
    }
#if 0
    if (flags & SSCAN_ERROR)
	printf("# ");
#endif
    printf("}\n");
    if ((flags & SSCAN_ERROR) == 0 && error) {
	fseek(stdout, off, 0);
	DumpDStructs(type, name, flags|SSCAN_ERROR);
    }
}

