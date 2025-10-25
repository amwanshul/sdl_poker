"""
Professional Placeholder Card Generator for SDL Poker Game
Creates high-quality, visually appealing card images
Requires: pip install pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os

# Create res directory if it doesn't exist
os.makedirs('res', exist_ok=True)

# Card dimensions
CARD_WIDTH = 80
CARD_HEIGHT = 120

# Suits and ranks
suits = {'H': '♥', 'D': '♦', 'C': '♣', 'S': '♠'}
suit_names = {'H': 'Hearts', 'D': 'Diamonds', 'C': 'Clubs', 'S': 'Spades'}
suit_colors = {
    'H': (220, 20, 60),    # Crimson
    'D': (255, 69, 0),     # Red-Orange
    'C': (25, 25, 25),     # Near Black
    'S': (0, 0, 0)         # Black
}
ranks = ['2', '3', '4', '5', '6', '7', '8', '9', '10', 'J', 'Q', 'K', 'A']
rank_names = {
    '2': 'Two', '3': 'Three', '4': 'Four', '5': 'Five', '6': 'Six',
    '7': 'Seven', '8': 'Eight', '9': 'Nine', '10': 'Ten',
    'J': 'Jack', 'Q': 'Queen', 'K': 'King', 'A': 'Ace'
}

def get_font(size):
    """Try to get a nice font, fall back to default if unavailable"""
    try:
        # Try common system fonts
        return ImageFont.truetype("arial.ttf", size)
    except:
        try:
            return ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", size)
        except:
            try:
                return ImageFont.truetype("C:\\Windows\\Fonts\\arial.ttf", size)
            except:
                return ImageFont.load_default()

def create_card(suit_code, rank):
    """Create a professional-looking card image"""
    # Create card with subtle gradient effect (simulate with layered rectangles)
    img = Image.new('RGB', (CARD_WIDTH, CARD_HEIGHT), 'white')
    draw = ImageDraw.Draw(img)
    
    # Draw rounded rectangle effect (corners)
    corner_radius = 5
    for i in range(corner_radius):
        alpha = int(255 * (1 - i / corner_radius))
        gray = (240 - i * 20, 240 - i * 20, 240 - i * 20)
        
        # Top-left corner
        draw.rectangle([i, i, corner_radius, corner_radius], fill=gray)
        # Top-right corner
        draw.rectangle([CARD_WIDTH - corner_radius, i, CARD_WIDTH - i, corner_radius], fill=gray)
        # Bottom-left corner
        draw.rectangle([i, CARD_HEIGHT - corner_radius, corner_radius, CARD_HEIGHT - i], fill=gray)
        # Bottom-right corner
        draw.rectangle([CARD_WIDTH - corner_radius, CARD_HEIGHT - corner_radius, 
                       CARD_WIDTH - i, CARD_HEIGHT - i], fill=gray)
    
    # Main white rectangle
    draw.rectangle([3, 3, CARD_WIDTH - 4, CARD_HEIGHT - 4], fill='white', outline=(200, 200, 200), width=1)
    
    # Draw outer border with slight shadow effect
    draw.rectangle([0, 0, CARD_WIDTH - 1, CARD_HEIGHT - 1], outline=(100, 100, 100), width=2)
    draw.rectangle([1, 1, CARD_WIDTH - 2, CARD_HEIGHT - 2], outline=(180, 180, 180), width=1)
    
    # Get suit symbol and color
    suit_symbol = suits[suit_code]
    color = suit_colors[suit_code]
    
    # Load fonts
    rank_font = get_font(16)
    suit_font = get_font(32)
    small_font = get_font(10)
    
    # Draw rank in top-left corner
    draw.text((6, 2), rank, fill=color, font=rank_font)
    draw.text((6, 18), suit_symbol, fill=color, font=small_font)
    
    # Draw large suit symbol in center
    # Calculate center position
    suit_bbox = draw.textbbox((0, 0), suit_symbol, font=suit_font)
    suit_width = suit_bbox[2] - suit_bbox[0]
    suit_height = suit_bbox[3] - suit_bbox[1]
    center_x = (CARD_WIDTH - suit_width) // 2
    center_y = (CARD_HEIGHT - suit_height) // 2
    
    draw.text((center_x, center_y), suit_symbol, fill=color, font=suit_font)
    
    # Draw rank below center suit
    rank_bbox = draw.textbbox((0, 0), rank, font=rank_font)
    rank_width = rank_bbox[2] - rank_bbox[0]
    draw.text(((CARD_WIDTH - rank_width) // 2, center_y + 35), rank, fill=color, font=rank_font)
    
    # Draw rank and suit in bottom-right corner (rotated effect via positioning)
    draw.text((CARD_WIDTH - 20, CARD_HEIGHT - 30), rank, fill=color, font=rank_font)
    draw.text((CARD_WIDTH - 20, CARD_HEIGHT - 15), suit_symbol, fill=color, font=small_font)
    
    # Add small decorative corners
    corner_color = (240, 240, 240)
    draw.rectangle([2, 2, 4, 4], fill=corner_color)
    draw.rectangle([CARD_WIDTH - 5, 2, CARD_WIDTH - 3, 4], fill=corner_color)
    draw.rectangle([2, CARD_HEIGHT - 5, 4, CARD_HEIGHT - 3], fill=corner_color)
    draw.rectangle([CARD_WIDTH - 5, CARD_HEIGHT - 5, CARD_WIDTH - 3, CARD_HEIGHT - 3], fill=corner_color)
    
    return img

def create_card_back():
    """Create an attractive card back design"""
    # Rich blue background
    img = Image.new('RGB', (CARD_WIDTH, CARD_HEIGHT), (25, 50, 120))
    draw = ImageDraw.Draw(img)
    
    # Outer border - gold/yellow
    draw.rectangle([0, 0, CARD_WIDTH - 1, CARD_HEIGHT - 1], outline=(218, 165, 32), width=3)
    draw.rectangle([3, 3, CARD_WIDTH - 4, CARD_HEIGHT - 4], outline=(255, 215, 0), width=1)
    
    # Inner decorative frame
    draw.rectangle([6, 6, CARD_WIDTH - 7, CARD_HEIGHT - 7], outline=(100, 149, 237), width=2)
    
    # Create diamond pattern
    pattern_color = (70, 130, 180)
    for y in range(10, CARD_HEIGHT - 10, 15):
        for x in range(10, CARD_WIDTH - 10, 15):
            # Draw small diamonds
            points = [
                (x, y - 4),      # Top
                (x + 4, y),      # Right
                (x, y + 4),      # Bottom
                (x - 4, y)       # Left
            ]
            draw.polygon(points, fill=pattern_color, outline=(100, 149, 237))
    
    # Center design - larger diamond
    center_x = CARD_WIDTH // 2
    center_y = CARD_HEIGHT // 2
    diamond_size = 15
    center_points = [
        (center_x, center_y - diamond_size),
        (center_x + diamond_size, center_y),
        (center_x, center_y + diamond_size),
        (center_x - diamond_size, center_y)
    ]
    draw.polygon(center_points, fill=(255, 215, 0), outline=(218, 165, 32), width=2)
    
    # Add some accent lines
    for i in range(12, CARD_WIDTH - 12, 8):
        draw.line([(i, 10), (i, CARD_HEIGHT - 10)], fill=(50, 80, 140), width=1)
    
    return img

def create_table_background():
    """Create a professional poker table background"""
    img = Image.new('RGB', (1024, 768), (0, 120, 50))  # Rich green felt
    draw = ImageDraw.Draw(img)
    
    # Create gradient effect (darker at edges)
    for i in range(100):
        darkness = int(i * 0.4)
        color = (0, max(0, 120 - darkness), max(0, 50 - darkness // 2))
        
        # Draw concentric rectangles
        draw.rectangle([i, i, 1024 - i, 768 - i], outline=color)
    
    # Draw elegant ellipse table outline
    table_color = (139, 69, 19)  # Saddle brown for wood
    
    # Outer wood rim (thick)
    draw.ellipse([40, 40, 984, 728], outline=table_color, width=15)
    
    # Inner wood highlight
    draw.ellipse([42, 42, 982, 726], outline=(160, 82, 45), width=2)
    
    # Outer edge shadow
    draw.ellipse([38, 38, 986, 730], outline=(90, 50, 15), width=3)
    
    # Add decorative corner marks
    corner_positions = [
        (100, 100), (924, 100),  # Top corners
        (100, 668), (924, 668)   # Bottom corners
    ]
    
    for cx, cy in corner_positions:
        # Small circles in corners
        draw.ellipse([cx - 10, cy - 10, cx + 10, cy + 10], 
                    outline=(255, 215, 0), width=2)
        draw.ellipse([cx - 7, cy - 7, cx + 7, cy + 7], 
                    fill=(0, 100, 40), outline=(200, 200, 150))
    
    # Add center dealer button area
    center_x, center_y = 512, 384
    draw.ellipse([center_x - 40, center_y - 30, center_x + 40, center_y + 30],
                fill=(0, 100, 40), outline=(255, 215, 0), width=2)
    
    # Try to add "POKER" text in center
    try:
        font = get_font(20)
        text = "POKER"
        bbox = draw.textbbox((0, 0), text, font=font)
        text_width = bbox[2] - bbox[0]
        draw.text((center_x - text_width // 2, center_y - 10), text, 
                 fill=(255, 215, 0), font=font)
    except:
        pass  # Skip text if font fails
    
    return img

# Generate all 52 cards with progress
print("=" * 50)
print("  PROFESSIONAL POKER CARD GENERATOR")
print("=" * 50)
print("\n🎴 Generating card images...\n")

card_count = 0
for suit_code in suits.keys():
    print(f"  Creating {suit_names[suit_code]}...", end=" ")
    for rank in ranks:
        filename = f"res/{suit_code}{rank}.png"
        card_img = create_card(suit_code, rank)
        card_img.save(filename)
        card_count += 1
    print(f"✓ ({len(ranks)} cards)")

print(f"\n✅ Created {card_count} card face images")

# Generate card back
print("\n🎴 Generating card back...", end=" ")
card_back = create_card_back()
card_back.save('res/card_back.png')
print("✓")

# Generate table background
print("🎴 Generating poker table...", end=" ")
table = create_table_background()
table.save('res/table.png')
print("✓")

print("\n" + "=" * 50)
print("  ✅ ALL ASSETS GENERATED SUCCESSFULLY!")
print("=" * 50)
print(f"\n📁 Location: ./res/ directory")
print(f"📊 Total files: {card_count + 2}")
print(f"\n🎮 You can now compile and run your poker game:")
print("   make")
print("   ./sdl_poker.exe")
print("\n" + "=" * 50)