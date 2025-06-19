# 🎬 GET_SPLATS.md - Your Complete Guide to Gaussian Splatting Files

This guide covers everything you need to know about getting 3D and 4D Gaussian Splatting (.ply) files for XRGS.

## 🚀 Quick Start

**Use our download script**:
```bash
./download_splats.sh
```

This interactive script will help you download popular datasets automatically!

---

## 🎬 **4D Gaussian Splatting (.ply videos)**

4DGS files contain temporal data - scenes that change over time (people moving, objects deforming).

### **🔥 Primary 4DGS Sources**

#### **1. 4DGaussians Official Dataset** ⭐
- **Repository**: https://github.com/hustvl/4DGaussians
- **Paper**: "4D Gaussian Splatting for Real-Time Dynamic Scene Rendering"
- **Content**: High-quality dynamic scenes
- **Scenes Available**:
  - `basketball` - Person shooting basketball (sports)
  - `boxes` - Moving boxes sequence (objects)
  - `football` - Football/soccer action (sports)
  - `jumpingjacks` - Person doing jumping jacks (exercise)
  - `playground` - Kids playing (human motion)
  - `sear_steak` - Cooking steak (deformation)
  - `space_out` - Artistic scene (abstract)
  - `spin` - Spinning object (rotation)

#### **2. D-NeRF Dataset (4DGS Compatible)**
- **Original**: https://github.com/albertpumarola/D-NeRF
- **4DGS Versions**: Available through 4DGaussians repo
- **Content**: Synthetic dynamic scenes
- **Scenes**: `mutant`, `standup`, `jumpingjacks`, `hellwarrior`, `hook`, `bouncing_balls`, `lego`, `trex`

#### **3. HyperNeRF Dataset (4DGS Compatible)**
- **Repository**: https://github.com/google/hypernerf
- **Content**: Real-world deformable scenes
- **Scenes**: Faces, hands, cloth, soft materials
- **Note**: Requires conversion pipeline

#### **4. Immersive Video Dataset**
- **Source**: Various research institutions
- **Content**: Multi-view synchronized video captures
- **Applications**: Sports, concerts, social interactions

### **📥 4DGS Download Examples**
```bash
# Clone 4DGaussians repository
git clone https://github.com/hustvl/4DGaussians.git
cd 4DGaussians

# Download sample data (following their instructions)
bash scripts/download_sample_data.sh

# Or use our automated script
./download_splats.sh
```

---

## 🏠 **3D Gaussian Splatting (.ply static scenes)**

3DGS files contain static 3D scenes - perfect for architecture, objects, and landscapes.

### **🌟 Popular 3DGS Datasets**

#### **1. Mip-NeRF 360 Dataset** ⭐ (Most Popular)
- **Source**: https://jonbarron.info/mipnerf360/
- **Quality**: Extremely high-quality real scenes
- **Outdoor Scenes**:
  - `bicycle` - Vintage bicycle in garden
  - `flowers` - Colorful flower arrangement  
  - `garden` - Lush garden scene
  - `stump` - Tree stump with surroundings
  - `treehill` - Hillside with trees
- **Indoor Scenes**:
  - `room` - Living room interior
  - `counter` - Kitchen counter scene
  - `kitchen` - Full kitchen view
  - `bonsai` - Bonsai tree on table

#### **2. NeRF Synthetic Dataset** (Blender)
- **Source**: https://drive.google.com/drive/folders/128yBriW1IG_3NJ5Rp7APSTZsJqdJdfc1
- **Content**: Synthetic objects with clean backgrounds
- **Objects**:
  - `chair` - Wooden chair
  - `drums` - Drum set
  - `ficus` - Plant in pot
  - `hotdog` - Hot dog
  - `lego` - LEGO bulldozer
  - `materials` - Spheres with different materials
  - `mic` - Vintage microphone
  - `ship` - Pirate ship

#### **3. Tanks and Temples Dataset**
- **Source**: https://www.tanksandtemples.org/
- **Content**: Large-scale outdoor reconstructions
- **Quality**: Photorealistic historical sites and landmarks
- **Scenes**: Temples, churches, historical buildings

#### **4. DTU Dataset**
- **Source**: https://roboimagedata.compute.dtu.dk/
- **Content**: Multi-view stereo benchmark objects
- **Quality**: High-precision indoor objects
- **Use**: Good for testing and benchmarking

### **🤗 Community Sources**

#### **1. Hugging Face Collections**
- **URL**: https://huggingface.co/collections/gaussian-splatting
- **Content**: Community-uploaded scenes
- **Variety**: Everything from pets to landmarks
- **Download**: Direct links, easy access

#### **2. Polycam Gallery**
- **URL**: https://poly.cam/explore
- **Content**: iPhone/Android LiDAR captures
- **Format**: Can export as Gaussian Splatting
- **Quality**: Good for real-world objects

#### **3. Luma AI Gallery**
- **URL**: https://lumalabs.ai/gallery
- **Content**: NeRF captures convertible to GS
- **Quality**: High-quality smartphone captures

#### **4. Research Institution Datasets**
- **Max Planck Institute**: Various computer vision datasets
- **MIT CSAIL**: Urban and indoor scenes
- **Stanford**: 3D scanning datasets
- **UC Berkeley**: Outdoor scene collections

---

## 🛠️ **Create Your Own Gaussian Splatting Files**

### **📱 Mobile Apps (Easiest)**

#### **Polycam** (iOS/Android)
1. Download Polycam app
2. Use "Gaussian Splatting" mode
3. Capture your scene (30-100 photos)
4. Wait for processing
5. Export as .ply file
6. Transfer to your computer

#### **Luma AI** (iOS)
1. Download Luma AI app
2. Capture scene with video mode
3. Upload and wait for processing
4. Download as NeRF or Gaussian Splatting

#### **Reality Capture** (Professional)
- Desktop software for high-quality reconstruction
- Supports DSLR camera inputs
- Professional results

### **🖥️ Desktop Pipeline (Advanced)**

#### **COLMAP + 3D Gaussian Splatting**
```bash
# 1. Install COLMAP
sudo apt install colmap

# 2. Clone 3D Gaussian Splatting
git clone https://github.com/graphdeco-inria/gaussian-splatting.git
cd gaussian-splatting

# 3. Process your photos
python train.py -s /path/to/your/images -m /path/to/output

# 4. Get your .ply file from output folder
```

#### **Nerfstudio Pipeline**
```bash
# Modern NeRF → Gaussian Splatting pipeline
pip install nerfstudio
ns-train gaussian-splatting --data /path/to/images
```

---

## 📂 **Recommended File Organization**

```
~/gaussian_scenes/
├── 3dgs_static/           # Static 3D scenes
│   ├── mipnerf360/       # Mip-NeRF 360 dataset
│   ├── synthetic/        # NeRF synthetic objects
│   ├── tankstemples/     # Tanks and Temples
│   └── custom/           # Your own captures
├── 4dgs_dynamic/         # Dynamic 4D scenes
│   ├── 4dgaussians/      # 4DGaussians dataset
│   ├── dnerf/            # D-NeRF dataset
│   └── custom/           # Your own 4D captures
└── samples/              # Quick test files
```

Create this structure:
```bash
mkdir -p ~/gaussian_scenes/{3dgs_static/{mipnerf360,synthetic,tankstemples,custom},4dgs_dynamic/{4dgaussians,dnerf,custom},samples}
```

---

## 🎯 **Quick Start Downloads**

### **Beginner-Friendly Samples** (Small files, fast download)
1. **Flowers** (3D) - Beautiful, colorful, fast to render
2. **Lego** (3D) - Clean synthetic object
3. **Basketball** (4D) - Short human motion sequence

### **Showcase Quality** (Larger files, stunning results)
1. **Garden** (3D) - Photorealistic outdoor scene
2. **Kitchen** (3D) - Detailed indoor environment  
3. **Playground** (4D) - Complex human interactions

### **Testing & Development** (Medium complexity)
1. **Bicycle** (3D) - Good for testing outdoor algorithms
2. **Bonsai** (3D) - Detailed object with complex geometry
3. **Boxes** (4D) - Simple motion for algorithm testing

---

## 🔗 **Direct Download Links**

### **Pre-processed Ready-to-Use Files**

#### **Mip-NeRF 360 (3DGS converted)**
```bash
# Example downloads (use our script for easier access)
wget https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/tandt_db.zip
wget https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/truck.zip
```

#### **Sample 4DGS Files**
```bash
# Use our download_splats.sh script for automated downloads
./download_splats.sh
```

---

## 🎮 **XRGS-Specific Tips**

### **Best Performance** (Your RTX 3060 Ti)
- **3DGS**: Any scene up to 2M splats runs smoothly
- **4DGS**: Sequences up to 1M splats per frame
- **Recommended**: Start with Mip-NeRF 360 scenes

### **VR Mode Considerations**
- Use smaller scenes (< 1M splats) for VR
- Prefer well-lit scenes for better VR experience
- Indoor scenes often work better than outdoor

### **File Size Guide**
- **Small**: < 50MB (quick testing)
- **Medium**: 50-200MB (good balance)
- **Large**: 200MB-1GB (showcase quality)
- **Huge**: > 1GB (research/development)

---

## 🚨 **Troubleshooting**

### **Download Issues**
- Use our `download_splats.sh` script for automated handling
- Some datasets require academic registration
- Large files may need wget/curl with resume capability

### **File Format Issues**
- XRGS supports .ply format specifically
- Some downloads come as .zip - extract first
- Ensure .ply files are in Gaussian Splatting format, not just point clouds

### **Performance Issues**
- Start with smaller scenes if XRGS is slow
- Check GPU memory usage with `nvidia-smi`
- Consider reducing scene complexity for real-time performance

---

## 📊 **Dataset Comparison**

| Dataset | Type | Scenes | Quality | Size | Best For |
|---------|------|--------|---------|------|----------|
| Mip-NeRF 360 | 3DGS | 9 scenes | ⭐⭐⭐⭐⭐ | Medium | Photorealism |
| NeRF Synthetic | 3DGS | 8 objects | ⭐⭐⭐⭐ | Small | Development |
| 4DGaussians | 4DGS | 8+ scenes | ⭐⭐⭐⭐⭐ | Large | Motion/VR |
| Tanks & Temples | 3DGS | 14 scenes | ⭐⭐⭐⭐⭐ | Large | Architecture |
| D-NeRF | 4DGS | 8 scenes | ⭐⭐⭐ | Medium | Synthetic Motion |

---

## 🎓 **Advanced: Dataset Creation**

### **For Researchers/Developers**
1. **Multi-view Photography**:
   - 50-150 photos from different angles
   - Good lighting, avoid shadows
   - Overlap between photos: 60-80%

2. **Video Capture (4DGS)**:
   - Multi-camera synchronized setup
   - High framerate (60fps+)
   - Consistent lighting

3. **Processing Pipeline**:
   - COLMAP for camera poses
   - 3DGS/4DGS training scripts
   - Optimization and cleanup

### **Quality Tips**
- **Lighting**: Avoid harsh shadows
- **Textures**: Rich surface details help
- **Motion**: Smooth, predictable movement for 4DGS
- **Backgrounds**: Clean backgrounds process better

---

## 📝 **Contributing**

Found a great dataset? Help the community:

1. Test it with XRGS
2. Document performance on your system
3. Add download instructions
4. Submit a pull request to this guide

---

## 🔗 **Additional Resources**

- **Papers**: https://github.com/MrNeRF/awesome-3D-gaussian-splatting
- **Tools**: https://github.com/graphdeco-inria/gaussian-splatting
- **Community**: Reddit r/MachineLearning, Twitter #GaussianSplatting
- **Courses**: YouTube "Gaussian Splatting Explained"

---

**Last Updated**: December 2024  
**XRGS Version**: Linux Port (commit aca2a1b)  
**Tested On**: Linux Mint 21.2, RTX 3060 Ti 