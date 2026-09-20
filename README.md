# LaminaSort

Client-side inventory sorting and management mod for Minecraft Bedrock Edition,
built on [LeviLamina Client](https://github.com/LiteLDev/LeviLamina).

## Target

- LeviLamina **v26.51.1** (client)
- Minecraft Bedrock Edition **1.26.51.x** (Windows x64)

## Building

Requirements: xmake, Visual Studio 2022 build tools, and LLVM/clang-cl.

```shell
xmake f -y -p windows -a x64 -m release --target_type=client
xmake
```

## License

[MIT](LICENSE) © amatouhake

Bootstrapped from the CC0-1.0 licensed
[levilamina-mod-template](https://github.com/LiteLDev/levilamina-mod-template)
