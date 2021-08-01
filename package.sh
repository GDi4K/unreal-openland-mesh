echo "Packaging ..."

rm -rf ToPackage

mkdir -p ToPackage/OpenLandMesh

cp OpenLandMeshDevApp/Plugins/OpenLandMesh/OpenLandMesh.uplugin ToPackage/OpenLandMesh/OpenLandMesh.uplugin

cp -rf OpenLandMeshDevApp/Plugins/OpenLandMesh/Config ToPackage/OpenLandMesh
cp -rf OpenLandMeshDevApp/Plugins/OpenLandMesh/Content ToPackage/OpenLandMesh
cp -rf OpenLandMeshDevApp/Plugins/OpenLandMesh/Source ToPackage/OpenLandMesh
cp -rf OpenLandMeshDevApp/Plugins/OpenLandMesh/Resources ToPackage/OpenLandMesh       