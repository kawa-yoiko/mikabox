# awk -v output=<c|wren<0|1|2>> -f syscalls.awk syscalls.txt
# :> wren_bind_mikabox.c; for i in 0 1 2; do awk -v output=wren$i -f syscalls.awk syscalls.txt >> wren_bind_mikabox.c; done

BEGIN {
  FS = ","

  type_c["_"] = "void"
  type_c["u16"] = "uint16_t"
  type_c["u32"] = "uint32_t"
  type_c["u64"] = "uint64_t"
  type_c["f32"] = "float"
  type_c["ptr"] = "void *"
  type_c["cptr"] = "const void *"

  if (output == "wren0") {
    printf("#include \"main.h\"\n")
    printf("#include <string.h>\n\n")
    printf("const char *wren_def_mikabox =\n")
    printf("\"class Mikabox {\\n\"\n")
  } else if (output == "wren2") {
    printf("WrenForeignMethodFn wren_bind_mikabox(WrenVM *vm, const char *module,\n")
    printf("  const char *class_name, bool is_static, const char *signature)\n")
    printf("{\n")
    printf("  if (is_static && strcmp(class_name, \"Mikabox\") == 0) {\n")
  }
}

function underscore_to_camel(s,   result, n, i, last_und) {
  result = ""
  n = split(s, s, "")
  last_und = 0
  for (i = 1; i <= length(s); i++) {
    if (s[i] == "_") {
      last_und = 1
    } else if (last_und) {
      last_und = 0
      result = result toupper(s[i])
    } else {
      result = result s[i]
    }
  }
  return result
}

$1 ~ /^ *[0-9]+ *$/ {
  n = split($2, def, " ")

  if (n == 3 && def[1] == "==" && def[3] == "==") {
    # Scope change
    scope_offset = $1
    scope = def[2]
  } else if (NF >= 2) {
    # Definition
    type[0] = def[1]
    name[0] = def[2]

    for (i = 3; i <= NF; i++) {
      m = split($i, arg, " ")
      if (m >= 2) {
        type[i - 2] = arg[1]
        name[i - 2] = arg[2]
        aux[i - 2] = arg[3]
        for (j = 4; j <= m; j++)
          aux[i - 2] = aux[i - 2] " " arg[j]
      }
    }
    argc = NF - 2

    if (output == "c") {
      printf("static inline %s %s_%s(", type_c[type[0]], scope, name[0])
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("%s%s%s", type_c[type[i]], (type[i] ~ "ptr") ? "" : " ", name[i])
      }
      printf(") { ")
      if (type[0] != "_") printf("return ")
      printf("syscall%d(", argc)
      for (i = 1; i <= argc; i++) {
        if (type[i] ~ "ptr") printf("(uint32_t)")
        printf("%s, ", name[i])
      }
      printf("%d); }\n", scope_offset + $1)
    } else if (output == "wren0") {
      printf("\"  foreign static %s(",
        underscore_to_camel((scope == "mika" ? "" : scope "_") name[0]))
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("%s", name[i])
      }
      printf(")\\n\"\n")
    } else if (output == "wren1") {
      printf("static void wren_%s(WrenVM *vm)\n", scope "_" name[0])
      printf("{\n")

      # Take arguments from Wren
      outbuf = 0
      for (i = 1; i <= argc; i++) {
        is_ptr = (type[i] ~ "ptr")
        wren_type_1 = (is_ptr ? (aux[i] ? "LIST" : "STRING") : "NUM")
        printf("  if (wrenGetSlotType(vm, %d) != WREN_TYPE_%s)\n", i, wren_type_1)
        printf("    printf(\"Argument %d has incorrect type\");\n", i)
        if (is_ptr && aux[i]) {
          if (aux[i] ~ "^out ") {
            outbuf = i
          } else {
            elm_type = type_c[aux[i]]
            printf("  wrenEnsureSlots(vm, %d);\n", argc + 1)
            printf("  int n%d = wrenGetListCount(vm, %d);\n", i, i)
            printf("  %s *%s = malloc(sizeof(%s) * n%d);\n", elm_type, name[i], elm_type, i)
            printf("  for (int i = 0; i < n%d; i++) {\n", i)
            printf("    wrenGetListElement(vm, %d, i, %d);\n", i, argc + 1)
            printf("    %s[i] = (%s)wrenGetSlotDouble(vm, %d);\n", name[i], elm_type, argc + 1)
            printf("  }\n")
          }
          printf("\n")
        } else {
          wren_type_2 = (is_ptr ? "String" : "Double")
          c_type = (is_ptr ? "const void *" : "double ")
          printf("  %s%s = wrenGetSlot%s(vm, %d);\n\n",
            c_type, name[i], wren_type_2, i)
        }
      }

      # Handle output buffers
      if (outbuf != 0) {
        # Remove "out " prefix and use as memory size
        printf("  char *%s = malloc((size_t)%s);\n", name[outbuf], substr(aux[outbuf], 5))
      }

      # The real call
      printf("  ")
      if (type[0] != "_") printf("double ret = (double)")
      printf("%s_%s(", scope, name[0])
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(", ")
        printf("(%s)%s", type_c[type[i]], name[i])
      }
      printf(");\n")

      # Pass the return value to Wren
      # Output buffers are put inside a list at slot index `outbuf`
      if (outbuf != 0) {
        printf("\n")
        size = substr(aux[outbuf], 5)
        if (size ~ "^[0-9]+$")
          printf("  wrenSetSlotString(vm, 0, %s);\n", name[outbuf])
        else printf("  wrenSetSlotBytes(vm, 0, %s, (size_t)%s);\n", name[outbuf], size)
        printf("  wrenInsertInList(vm, %d, -1, 0);\n", outbuf)
      }
      # Return value
      if (type[0] != "_") printf("  wrenSetSlotDouble(vm, 0, ret);\n")

      # Free memory allocated
      for (i = 1; i <= argc; i++)
        if (type[i] ~ "ptr" && aux[i]) {
          printf("  free(%s);\n", name[i])
        }
      printf("}\n\n")
    } else if (output == "wren2") {
      printf("    if (strcmp(signature, \"%s(",
        underscore_to_camel((scope == "mika" ? "" : scope "_") name[0]))
      for (i = 1; i <= argc; i++) {
        if (i > 1) printf(",")
        printf("_")
      }
      printf(")\") == 0) ")
      printf("return wren_%s;\n", scope "_" name[0])
    }
  }
}

END {
  if (output == "wren0") {
    printf("\"}\\n\";\n\n")
  } else if (output == "wren2") {
    printf("  }\n")
    printf("\n")
    printf("  return NULL;\n")
    printf("}\n")
  }
}
