{
  "name": "AllGltfMeshComponents",
  "camera": "MeshTestCamera",
  "objects": [
    {
      "name": "MeshTestCamera",
      "position": {
        "x": 0.0,
        "y": 4.0,
        "z": 18.0
      },
      "components": [
        {
          "type": "CameraComponent"
        }
      ]
    },
    {
      "name": "MeshTestLight",
      "position": {
        "x": 0.0,
        "y": 8.0,
        "z": 8.0
      },
      "components": [
        {
          "type": "LightComponent",
          "lightType": "directional",
          "color": {
            "r": 1.0,
            "g": 1.0,
            "b": 1.0
          }
        }
      ]
    },
    {
      "name": "Mesh_9mm",
      "position": {
        "x": -9.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/9mm/scene.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_CarbineGun",
      "position": {
        "x": -6.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/CarbineGun/scene.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_Door",
      "position": {
        "x": -3.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/Door/scene.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_IndustrinTerminal",
      "position": {
        "x": 0.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/IndustrinTerminal/chad_console.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_SurvivalBackpack",
      "position": {
        "x": 3.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/SurvivalBackpack/scene.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_Suzanne",
      "position": {
        "x": 6.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/suzanne/Suzanne.gltf"
          }
        }
      ]
    },
    {
      "name": "Mesh_TeleportationGate",
      "position": {
        "x": 9.0,
        "y": 1.0,
        "z": 0.0
      },
      "components": [
        {
          "type": "MeshComponent",
          "material": {
            "path": "Materials/Checker.mat"
          },
          "mesh": {
            "type": "file",
            "path": "Models/TeleportationGate/scene.gltf"
          }
        }
      ]
    }
  ]
}
