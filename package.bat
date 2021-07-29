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
