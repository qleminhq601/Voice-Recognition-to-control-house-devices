
const onButtons = document.querySelectorAll('.btn-on');
const offButtons = document.querySelectorAll('.btn-off');

// Function to turn the device on
function turnOn(device) {
  console.log(`${device} turned on`);
  const imageElement = document.querySelector(`img[alt='${device}']`);
  
  // Update the image based on the device
  if (device === "Light") {
    imageElement.src = "light-on.png";  // Change to light-on image
  } else if (device === "Fan") {
    imageElement.src = "fan-on.png";  // Change to fan-on image
  }
  // You can add additional logic to control the device state if needed
}

// Function to turn the device off
function turnOff(device) {
  console.log(`${device} turned off`);
  const imageElement = document.querySelector(`img[alt='${device}']`);
  
  // Update the image based on the device
  if (device === "Light") {
    imageElement.src = "light-off.png";  // Change to light-off image
  } else if (device === "Fan") {
    imageElement.src = "fan-off.png";  // Change to fan-off image (for Fan)
  }
  // You can add additional logic to control the device state if needed
}

// Attach event listeners to 'On' buttons
onButtons.forEach((button, index) => {
  button.addEventListener('click', () => {
    // Find the device name (e.g., Light, Fan)
    const device = button.closest('.grid-item').querySelector('img').alt;
    turnOn(device);
  });
});

// Attach event listeners to 'Off' buttons
offButtons.forEach((button, index) => {
  button.addEventListener('click', () => {
    // Find the device name (e.g., Light, Fan)
    const device = button.closest('.grid-item').querySelector('img').alt;
    turnOff(device);
  });
});
