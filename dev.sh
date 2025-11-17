#!/bin/bash
# ROS2 Development Container Helper Script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Export current user's UID/GID for docker-compose
export USER_UID=$(id -u)
export USER_GID=$(id -g)
export USERNAME=$(id -un)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if container is running
is_running() {
    docker compose ps -q ros2-dev | grep -q .
}

# Function to build the container
build() {
    print_info "Building ROS2 development container..."
    docker compose build
    print_info "Build complete!"
}

# Function to start the container
start() {
    if is_running; then
        print_warn "Container is already running"
    else
        print_info "Starting ROS2 development container..."
        docker compose up -d
        print_info "Container started!"
    fi
}

# Function to stop the container
stop() {
    if is_running; then
        print_info "Stopping ROS2 development container..."
        docker compose down
        print_info "Container stopped!"
    else
        print_warn "Container is not running"
    fi
}

# Function to enter the container
shell() {
    if ! is_running; then
        print_warn "Container is not running. Starting it now..."
        start
        sleep 2
    fi
    print_info "Entering container shell..."
    docker compose exec -w /workspace/workspace ros2-dev /bin/bash
}

# Function to run a command in the container
run_cmd() {
    if ! is_running; then
        print_warn "Container is not running. Starting it now..."
        start
        sleep 2
    fi
    docker compose exec -w /workspace/workspace ros2-dev /bin/bash -c "source /opt/ros/\${ROS_DISTRO:-jazzy}/setup.bash && $*"
}

# Function to show logs
logs() {
    docker compose logs -f ros2-dev
}

# Function to rebuild and restart
rebuild() {
    print_info "Rebuilding container..."
    stop
    build
    start
    print_info "Container rebuilt and started!"
}

# Function to show status
status() {
    if is_running; then
        print_info "Container is running"
        docker compose ps
    else
        print_warn "Container is not running"
    fi
}

# Function to clean up
clean() {
    print_warn "This will remove the container and volumes. Continue? (y/N)"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        docker compose down -v
        print_info "Cleanup complete!"
    else
        print_info "Cleanup cancelled"
    fi
}

# Show help
show_help() {
    cat << EOF
ROS2 Development Container Helper

Usage: $0 [command]

Commands:
    build       Build the container image
    start       Start the container in the background
    stop        Stop the container
    shell       Open a bash shell in the container
    run         Run a command in the container
    logs        Show container logs
    rebuild     Rebuild and restart the container
    status      Show container status
    clean       Remove container and volumes
    help        Show this help message

Examples:
    $0 build                    # Build the container
    $0 shell                    # Enter the container
    $0 run colcon build         # Run colcon build in the container
    $0 run ros2 topic list      # List ROS2 topics

EOF
}

# Main script
case "${1:-help}" in
    build)
        build
        ;;
    start)
        start
        ;;
    stop)
        stop
        ;;
    shell)
        shell
        ;;
    run)
        shift
        run_cmd "$@"
        ;;
    logs)
        logs
        ;;
    rebuild)
        rebuild
        ;;
    status)
        status
        ;;
    clean)
        clean
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        print_error "Unknown command: $1"
        show_help
        exit 1
        ;;
esac
