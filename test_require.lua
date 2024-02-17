require("test_module_1")
print("test_require")
print("========== package.loaded ==========")
for name, module in pairs(package.loaded) do
    print(name, module)
end
print("========== package.loaders ==========")
for name, func in pairs(package.loaders) do
    print(name, module)
end
