import { OpenAPI, UsersService, ApiError, MessageResponse } from '@/src/api';

async function main() {
    try {
        const user = await UsersService.registerUser({
            username: 'testuser',
            email: 'testuser@example.com',
            password: 'password'
        });
        console.log('Registered user:', user);
    } catch (error) {
        if (error instanceof ApiError) {
            const message = (error.body as MessageResponse).detail;
            console.error('Error registering user:', message);
        } else {
            console.error('Unexpected error:', error);
        }
    }
}

async function testLogin() {
    try {
        const token = await UsersService.loginUser({
            username: 'admin_iot_hidroponik',
            password: 'superadmin1234'
        });
        console.log('Login successful, token:', token);

        OpenAPI.TOKEN = token.access_token;

        const listAccount = await UsersService.getAllUsers();
        console.log('List of accounts:', listAccount);
    } catch (error) {
        if (error instanceof ApiError) {
            const message = (error.body as MessageResponse).detail;
            console.error('Error logging in:', message);
        } else {
            console.error('Unexpected error:', error);
        }
    }
}

main();
testLogin();