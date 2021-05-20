## What is this?

This is a Unreal Engine 4 Plugin that export ue4 navigation mesh data(recast mesh) to outside.

With this plugin, you can export recast Navigation data directly from the UE without going through RecastDemo.Of course I also kept the export recast navmesh.

### How do use?

1.  add the plugin to the project and enable it. 
2.  Launch the Project in Editor, Click the `ExportNav` button. It will create to a `.bin` and `.obj` file.

- The `.bin` file is the export recast navigation data that is directly from the UE.You can use it in `detour`.
- The `.obj` file is a navigation mesh exported from the UE(unit is centimeter).

### Use .obj

![](https://img.imzlp.com/imgs/zlp/blog/notes/ue/index/UE4/Plugins/export-nav-data/ue4-export-nav-data-usage-0.png)

![](https://img.imzlp.com/imgs/zlp/blog/notes/ue/index/UE4/Plugins/export-nav-data/ue4-export-nav-data-usage-1.png)

3. Open The Plugin `Source/ExportNav/ThirdParty/RecastDemoBin`
4. copy `.obj` to `RecastDemoBin/Meshes`
5. run `RecastDemo.exe`

![](https://img.imzlp.com/imgs/zlp/blog/notes/ue/index/UE4/Plugins/export-nav-data/Recast-Demo-bin.png)

