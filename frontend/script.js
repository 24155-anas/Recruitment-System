// Toggle between Login and Signup forms
function showLogin() {
    document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
    document.querySelectorAll('.auth-form').forEach(form => form.classList.remove('active'));
    
    document.querySelector('.tab:first-child').classList.add('active');
    document.getElementById('loginForm').classList.add('active');
}

function showSignup() {
    document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
    document.querySelectorAll('.auth-form').forEach(form => form.classList.remove('active'));
    
    document.querySelector('.tab:last-child').classList.add('active');
    document.getElementById('signupForm').classList.add('active');
}

// Show loading spinner
function showLoading() {
    document.getElementById('loading').style.display = 'block';
}

function hideLoading() {
    document.getElementById('loading').style.display = 'none';
}

// Show message
function showMessage(text, type) {
    const messageDiv = document.getElementById('message');
    messageDiv.textContent = text;
    messageDiv.className = `message ${type}`;
    messageDiv.style.display = 'block';
    
    // Auto hide after 5 seconds
    setTimeout(() => {
        messageDiv.style.display = 'none';
    }, 5000);
}

// Base URL for your C++ backend server
const API_BASE_URL = 'http://localhost:8080';

// Login Form Handler
document.getElementById('loginForm')?.addEventListener('submit', async function(e) {
    e.preventDefault();
    
    const email = document.getElementById('loginEmail').value;
    const password = document.getElementById('loginPassword').value;
    
    showLoading();
    
    try {
        const response = await fetch(`${API_BASE_URL}/api/login`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ email, password })
        });
        
        const data = await response.json();
        
        if (data.success) {
            showMessage('Login successful! Redirecting...', 'success');
            
            // Store user info in localStorage
            localStorage.setItem('user_email', email);
            localStorage.setItem('user_name', data.name || 'User');
            
            // Redirect to profile page after 1 second
            setTimeout(() => {
                window.location.href = 'profile.html';
            }, 1000);
        } else {
            showMessage(data.message || 'Login failed', 'error');
        }
    } catch (error) {
        showMessage('Cannot connect to server. Make sure backend is running.', 'error');
    } finally {
        hideLoading();
    }
});

// Signup Form Handler
document.getElementById('signupForm')?.addEventListener('submit', async function(e) {
    e.preventDefault();
    
    const email = document.getElementById('signupEmail').value;
    const password = document.getElementById('signupPassword').value;
    const confirmPassword = document.getElementById('confirmPassword').value;
    
    // Basic validation
    if (password !== confirmPassword) {
        showMessage('Passwords do not match!', 'error');
        return;
    }
    
    if (password.length < 6) {
        showMessage('Password must be at least 6 characters', 'error');
        return;
    }
    
    showLoading();
    
    try {
        const response = await fetch(`${API_BASE_URL}/api/signup`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ email, password, confirmPassword })
        });
        
        const data = await response.json();
        
        if (data.success) {
            showMessage('Account created successfully!', 'success');
            
            // Store user info
            localStorage.setItem('user_email', email);
            
            // Auto login and redirect after 1 second
            setTimeout(() => {
                window.location.href = 'profile.html';
            }, 1000);
        } else {
            showMessage(data.message || 'Signup failed', 'error');
        }
    } catch (error) {
        showMessage('Cannot connect to server. Make sure backend is running.', 'error');
    } finally {
        hideLoading();
    }
});

// CV Form Handler
document.getElementById('cvForm')?.addEventListener('submit', async function(e) {
    e.preventDefault();
    
    // Collect form data
    const cvData = {
        name: document.getElementById('fullName').value,
        email: document.getElementById('email').value,
        skills: document.getElementById('skills').value,
        experience: document.getElementById('experience').value,
        lastPosition: document.getElementById('lastPosition').value,
        education: document.getElementById('education').value || '',
        location: document.getElementById('location').value || '',
        phone: document.getElementById('phone').value || ''
    };
    
    // Basic validation
    if (!cvData.name || !cvData.email || !cvData.skills || !cvData.experience || !cvData.lastPosition) {
        showMessage('Please fill all required fields', 'error');
        return;
    }
    
    showLoading();
    
    try {
        const response = await fetch(`${API_BASE_URL}/api/save-cv`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(cvData)
        });
        
        const data = await response.json();
        
        if (data.success) {
            showMessage(`CV saved successfully! Record ID: ${data.id}`, 'success');
            
            // Clear form after successful save
            setTimeout(() => {
                clearForm();
            }, 2000);
        } else {
            showMessage('Failed to save data: ' + data.message, 'error');
        }
    } catch (error) {
        showMessage('Error connecting to database server. Please try again.', 'error');
    } finally {
        hideLoading();
    }
});

// Check server status on page load
async function checkServerStatus() {
    try {
        const response = await fetch(`${API_BASE_URL}/api/status`);
        const data = await response.json();
        console.log('Server status:', data);
    } catch (error) {
        console.log('Server is not running. Start the C++ backend server.');
    }
}

// Auto-fill email from localStorage on profile page
window.addEventListener('load', () => {
    checkServerStatus();
    
    // Auto-fill email if user is logged in
    const emailField = document.getElementById('email');
    if (emailField) {
        const savedEmail = localStorage.getItem('user_email');
        if (savedEmail) {
            emailField.value = savedEmail;
        }
    }
    
    // Auto-fill name if available
    const nameField = document.getElementById('fullName');
    if (nameField) {
        const savedName = localStorage.getItem('user_name');
        if (savedName) {
            nameField.value = savedName;
        }
    }
});
// Clear form function
function clearForm() {
    if (confirm('Are you sure you want to clear the form?')) {
        document.getElementById('cvForm').reset();
        showMessage('Form cleared', 'success');
    }
}

// Simulated API functions (Replace with actual fetch calls to your C++ backend)
function simulateLogin(email, password) {
    return new Promise((resolve) => {
        setTimeout(() => {
            // Demo login - accept any email/password
            resolve({
                success: true,
                message: 'Login successful',
                user: { email, name: 'Demo User' }
            });
        }, 1000);
    });
}

function simulateSignup(email, password) {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve({
                success: true,
                message: 'Account created successfully',
                user: { email }
            });
        }, 1000);
    });
}

function saveToDatabase(cvData) {
    return new Promise((resolve) => {
        setTimeout(() => {
            console.log('Sending to backend:', cvData);
            
            // In real implementation, send to your C++ backend
            // Example using Fetch API:
            /*
            fetch('http://localhost:8080/api/save-cv', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(cvData)
            })
            .then(response => response.json())
            .then(data => resolve(data))
            .catch(error => {
                resolve({ success: false, message: error.message });
            });
            */
            
            // Simulated success response
            resolve({
                success: true,
                message: 'Data stored in B-tree database',
                id: Math.floor(Math.random() * 1000) + 1,
                indexedAt: new Date().toISOString()
            });
        }, 1500);
    });
}

