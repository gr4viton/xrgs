#!/bin/bash

# XRGS Gaussian Splatting Dataset Downloader
# Interactive script to download popular 3D and 4D Gaussian Splatting datasets

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DEFAULT_DOWNLOAD_DIR="$HOME/gaussian_scenes"
TEMP_DIR="/tmp/gaussian_download"

# Create temp directory
mkdir -p "$TEMP_DIR"

echo -e "${BLUE}🎬 XRGS Gaussian Splatting Dataset Downloader${NC}"
echo "=============================================="
echo ""

# Function to create directory structure
create_directories() {
    local base_dir="$1"
    echo -e "${BLUE}📁 Creating directory structure at: $base_dir${NC}"
    mkdir -p "$base_dir"/{3dgs_static/{mipnerf360,synthetic,tankstemples,custom},4dgs_dynamic/{4dgaussians,dnerf,custom},samples}
    echo -e "${GREEN}✅ Directories created successfully${NC}"
}

# Function to download and extract file
download_extract() {
    local url="$1"
    local filename="$2"
    local extract_dir="$3"
    local description="$4"
    
    echo -e "${CYAN}🔽 Downloading: $description${NC}"
    echo "   URL: $url"
    echo "   Destination: $extract_dir"
    
    cd "$TEMP_DIR"
    
    # Download with resume capability
    if ! wget -c "$url" -O "$filename"; then
        echo -e "${RED}❌ Download failed for: $description${NC}"
        return 1
    fi
    
    # Extract if it's an archive
    if [[ "$filename" == *.zip ]]; then
        echo -e "${YELLOW}📦 Extracting ZIP archive...${NC}"
        unzip -o "$filename" -d "$extract_dir"
    elif [[ "$filename" == *.tar.gz ]] || [[ "$filename" == *.tgz ]]; then
        echo -e "${YELLOW}📦 Extracting TAR.GZ archive...${NC}"
        tar -xzf "$filename" -C "$extract_dir"
    elif [[ "$filename" == *.ply ]]; then
        echo -e "${YELLOW}📦 Moving PLY file...${NC}"
        mv "$filename" "$extract_dir/"
    else
        echo -e "${YELLOW}📦 Moving file...${NC}"
        mv "$filename" "$extract_dir/"
    fi
    
    echo -e "${GREEN}✅ Successfully downloaded and extracted: $description${NC}"
    echo ""
}

# Function to clone git repository
clone_repo() {
    local repo_url="$1"
    local target_dir="$2"
    local description="$3"
    
    echo -e "${CYAN}🔽 Cloning repository: $description${NC}"
    echo "   Repository: $repo_url"
    echo "   Destination: $target_dir"
    
    if git clone "$repo_url" "$target_dir"; then
        echo -e "${GREEN}✅ Successfully cloned: $description${NC}"
    else
        echo -e "${RED}❌ Failed to clone: $description${NC}"
        return 1
    fi
    echo ""
}

# Get download directory
echo -e "${YELLOW}📂 Where would you like to store the datasets?${NC}"
echo "   Default: $DEFAULT_DOWNLOAD_DIR"
read -p "   Enter path (or press Enter for default): " download_dir
download_dir="${download_dir:-$DEFAULT_DOWNLOAD_DIR}"

# Expand ~ to home directory
download_dir="${download_dir/#\~/$HOME}"

echo ""
echo -e "${BLUE}📁 Download directory: $download_dir${NC}"

# Create directories
create_directories "$download_dir"
echo ""

# Main menu
while true; do
    echo -e "${PURPLE}🎯 What would you like to download?${NC}"
    echo ""
    echo -e "${CYAN}3D GAUSSIAN SPLATTING (Static Scenes):${NC}"
    echo "  1) Mip-NeRF 360 Sample (flowers) - 45MB - Beginner friendly"
    echo "  2) Mip-NeRF 360 Complete Dataset - 2.1GB - All 9 scenes"
    echo "  3) NeRF Synthetic Samples (lego, chair) - 150MB - Clean objects"
    echo "  4) NeRF Synthetic Complete - 750MB - All 8 synthetic objects"
    echo ""
    echo -e "${CYAN}4D GAUSSIAN SPLATTING (Dynamic Scenes):${NC}"
    echo "  5) 4DGaussians Sample (basketball) - 200MB - Human motion"
    echo "  6) 4DGaussians Repository - Download full research repo"
    echo "  7) D-NeRF Sample (jumpingjacks) - 180MB - Synthetic motion"
    echo ""
    echo -e "${CYAN}QUICK START COLLECTIONS:${NC}"
    echo "  8) Essential Samples Pack - 400MB - Best starter files"
    echo "  9) Showcase Quality Pack - 1.2GB - High-quality demonstrations"
    echo " 10) Development Testing Pack - 600MB - Various complexity levels"
    echo ""
    echo -e "${CYAN}TOOLS & RESOURCES:${NC}"
    echo " 11) 3D Gaussian Splatting Training Code"
    echo " 12) 4D Gaussian Splatting Training Code"
    echo ""
    echo -e "${CYAN}UTILITIES:${NC}"
    echo " 13) Create empty project structure only"
    echo " 14) Show disk space requirements"
    echo ""
    echo "  0) Exit"
    echo ""
    
    read -p "Enter your choice (0-14): " choice
    echo ""
    
    case $choice in
        1)
            echo -e "${GREEN}📥 Downloading Mip-NeRF 360 Sample (flowers)${NC}"
            download_extract "https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/flowers.zip" \
                           "flowers.zip" \
                           "$download_dir/samples" \
                           "Flowers scene - Beautiful garden flowers"
            ;;
            
        2)
            echo -e "${GREEN}📥 Downloading Mip-NeRF 360 Complete Dataset${NC}"
            echo -e "${YELLOW}⚠️  This is a large download (2.1GB). Continue? (y/N)${NC}"
            read -p "" confirm
            if [[ "$confirm" =~ ^[Yy]$ ]]; then
                # Download all Mip-NeRF 360 scenes
                scenes=("bicycle" "flowers" "garden" "stump" "treehill" "room" "counter" "kitchen" "bonsai")
                for scene in "${scenes[@]}"; do
                    download_extract "https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/${scene}.zip" \
                                   "${scene}.zip" \
                                   "$download_dir/3dgs_static/mipnerf360" \
                                   "Mip-NeRF 360: $scene"
                done
            fi
            ;;
            
        3)
            echo -e "${GREEN}📥 Downloading NeRF Synthetic Samples${NC}"
            download_extract "https://drive.google.com/uc?export=download&id=1JDT-EqdbOp_sFUz8YmKBjNVZXF3EwFO_" \
                           "nerf_synthetic_lego.zip" \
                           "$download_dir/samples" \
                           "LEGO bulldozer - Clean synthetic object"
            download_extract "https://drive.google.com/uc?export=download&id=1JDT-EqdbOp_sFUz8YmKBjNVZXF3EwFO_" \
                           "nerf_synthetic_chair.zip" \
                           "$download_dir/samples" \
                           "Wooden chair - Clean synthetic object"
            ;;
            
        4)
            echo -e "${GREEN}📥 Downloading NeRF Synthetic Complete Dataset${NC}"
            echo -e "${YELLOW}⚠️  This will download all 8 synthetic objects (750MB). Continue? (y/N)${NC}"
            read -p "" confirm
            if [[ "$confirm" =~ ^[Yy]$ ]]; then
                echo -e "${BLUE}ℹ️  Note: Direct download links for NeRF Synthetic require manual setup${NC}"
                echo -e "   Please visit: https://drive.google.com/drive/folders/128yBriW1IG_3NJ5Rp7APSTZsJqdJdfc1"
                echo -e "   Download and extract to: $download_dir/3dgs_static/synthetic/"
            fi
            ;;
            
        5)
            echo -e "${GREEN}📥 Downloading 4DGaussians Sample (basketball)${NC}"
            clone_repo "https://github.com/hustvl/4DGaussians.git" \
                      "$download_dir/4dgs_dynamic/4dgaussians" \
                      "4DGaussians Repository with sample data"
            ;;
            
        6)
            echo -e "${GREEN}📥 Downloading 4DGaussians Repository${NC}"
            clone_repo "https://github.com/hustvl/4DGaussians.git" \
                      "$download_dir/4dgs_dynamic/4dgaussians" \
                      "Complete 4DGaussians Research Repository"
            echo -e "${BLUE}ℹ️  To download datasets, cd into the repo and follow their data instructions${NC}"
            ;;
            
        7)
            echo -e "${GREEN}📥 Downloading D-NeRF Sample${NC}"
            clone_repo "https://github.com/albertpumarola/D-NeRF.git" \
                      "$download_dir/4dgs_dynamic/dnerf" \
                      "D-NeRF Repository with sample data"
            ;;
            
        8)
            echo -e "${GREEN}📥 Downloading Essential Samples Pack${NC}"
            # Flowers (3D)
            download_extract "https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/flowers.zip" \
                           "flowers.zip" \
                           "$download_dir/samples" \
                           "Essential: Flowers (3D)"
            # Lego (3D) 
            echo -e "${BLUE}ℹ️  For synthetic objects, please download manually from NeRF Synthetic dataset${NC}"
            # Basketball (4D) - Clone 4DGaussians
            clone_repo "https://github.com/hustvl/4DGaussians.git" \
                      "$download_dir/samples/4dgaussians" \
                      "Essential: 4DGaussians samples"
            ;;
            
        9)
            echo -e "${GREEN}📥 Downloading Showcase Quality Pack${NC}"
            echo -e "${YELLOW}⚠️  This is a large download (1.2GB). Continue? (y/N)${NC}"
            read -p "" confirm
            if [[ "$confirm" =~ ^[Yy]$ ]]; then
                # Garden, Kitchen, Bicycle
                showcase_scenes=("garden" "kitchen" "bicycle")
                for scene in "${showcase_scenes[@]}"; do
                    download_extract "https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/${scene}.zip" \
                                   "${scene}.zip" \
                                   "$download_dir/samples/showcase" \
                                   "Showcase: $scene"
                done
            fi
            ;;
            
        10)
            echo -e "${GREEN}📥 Downloading Development Testing Pack${NC}"
            # Mixed complexity scenes for testing
            dev_scenes=("bicycle" "bonsai" "flowers")
            for scene in "${dev_scenes[@]}"; do
                download_extract "https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/datasets/input/${scene}.zip" \
                               "${scene}.zip" \
                               "$download_dir/samples/development" \
                               "Development: $scene"
            done
            ;;
            
        11)
            echo -e "${GREEN}📥 Downloading 3D Gaussian Splatting Training Code${NC}"
            clone_repo "https://github.com/graphdeco-inria/gaussian-splatting.git" \
                      "$download_dir/tools/gaussian-splatting" \
                      "Official 3D Gaussian Splatting Training Code"
            ;;
            
        12)
            echo -e "${GREEN}📥 Downloading 4D Gaussian Splatting Training Code${NC}"
            clone_repo "https://github.com/hustvl/4DGaussians.git" \
                      "$download_dir/tools/4DGaussians" \
                      "Official 4D Gaussian Splatting Training Code"
            ;;
            
        13)
            echo -e "${GREEN}📁 Creating empty project structure${NC}"
            create_directories "$download_dir"
            echo -e "${GREEN}✅ Empty structure created at: $download_dir${NC}"
            ;;
            
        14)
            echo -e "${BLUE}💾 Disk Space Requirements:${NC}"
            echo ""
            echo "  📁 Essential Samples Pack:    ~400MB"
            echo "  📁 Mip-NeRF 360 Complete:   ~2.1GB"
            echo "  📁 NeRF Synthetic Complete:  ~750MB"
            echo "  📁 Showcase Quality Pack:    ~1.2GB"
            echo "  📁 Development Testing:      ~600MB"
            echo "  📁 4DGaussians Repository:   ~500MB"
            echo ""
            echo "  🎯 Recommended for beginners: Essential Samples Pack"
            echo "  🏆 Best quality: Showcase Quality Pack"
            echo "  🔬 For research: Complete datasets"
            echo ""
            ;;
            
        0)
            echo -e "${GREEN}👋 Download session completed!${NC}"
            echo ""
            echo -e "${BLUE}📂 Your files are located at: $download_dir${NC}"
            echo -e "${BLUE}🎮 To run XRGS with a scene:${NC}"
            echo "   ./run.sh /path/to/scene.ply"
            echo ""
            echo -e "${BLUE}📖 For more information:${NC}"
            echo "   - Read GET_SPLATS.md for detailed dataset information"
            echo "   - Read README_LINUX.md for XRGS usage instructions"
            echo ""
            break
            ;;
            
        *)
            echo -e "${RED}❌ Invalid choice. Please enter a number between 0-14.${NC}"
            echo ""
            ;;
    esac
    
    # Ask if user wants to continue
    if [[ $choice != 0 && $choice != 14 ]]; then
        echo ""
        echo -e "${YELLOW}Continue downloading? (Y/n)${NC}"
        read -p "" continue_download
        if [[ "$continue_download" =~ ^[Nn]$ ]]; then
            echo -e "${GREEN}👋 Download session completed!${NC}"
            echo -e "${BLUE}📂 Your files are located at: $download_dir${NC}"
            break
        fi
        echo ""
    fi
done

# Cleanup temp directory
rm -rf "$TEMP_DIR"

echo -e "${GREEN}🎉 Happy Gaussian Splatting with XRGS!${NC}" 