echo "Packaging ..."

rmdir /S /Q ToPackage

@REM --- OpenLand ---
md ToPackage
md ToPackage\OpenLandMesh

copy OpenLandMeshDevApp\Plugins\OpenLandMesh\OpenLandMesh.uplugin ToPackage\OpenLandMesh\OpenLandMesh.uplugin /Y

md ToPackage\OpenLandMesh\Config
xcopy OpenLandMeshDevApp\Plugins\OpenLandMesh\Config ToPackage\OpenLandMesh\Config /E/H

md ToPackage\OpenLandMesh\Content
xcopy OpenLandMeshDevApp\Plugins\OpenLandMesh\Content ToPackage\OpenLandMesh\Content /E/H

md ToPackage\OpenLandMesh\Source
xcopy OpenLandMeshDevApp\Plugins\OpenLandMesh\Source ToPackage\OpenLandMesh\Source /E/H

md ToPackage\OpenLandMesh\Resources
xcopy OpenLandMeshDevApp\Plugins\OpenLandMesh\Resources ToPackage\OpenLandMesh\Resources /E/H

md ToPackage\OpenLandMesh\Binaries
md ToPackage\OpenLandMesh\Binaries\Win64
copy OpenLandMeshDevApp\Plugins\OpenLandMesh\Binaries\Win64\UE4Editor.modules ToPackage\OpenLandMesh\Binaries\Win64\UE4Editor.modules /Y
copy OpenLandMeshDevApp\Plugins\OpenLandMesh\Binaries\Win64\UE4Editor-OpenLandMesh.pdb ToPackage\OpenLandMesh\Binaries\Win64\UE4Editor-OpenLandMesh.pdb /Y
copy OpenLandMeshDevApp\Plugins\OpenLandMesh\Binaries\Win64\UE4Editor-OpenLandMesh.dll ToPackage\OpenLandMesh\Binaries\Win64\UE4Editor-OpenLandMesh.dll /Y