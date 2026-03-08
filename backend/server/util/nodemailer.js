import nodemailer from 'nodemailer'
import 'dotenv/config'

const transporter = nodemailer.createTransport({
	service: 'Gmail',
	host: 'smtp.gmail.com',
	port: 587,
	secure: false,
	auth: {
		user: process.env.GMAIL_USER,
		pass: process.env.GMAIL_PASSWORD,
	},
})

export async function sendEmail(toAddress, subject, text, html) {
	if (!process.env.GMAIL_USER || !process.env.GMAIL_PASSWORD) {
		throw new Error('No gmail config in .env!')
	}
	const mailOptions = {
		from: process.env.GMAIL_USER,
		to: toAddress,
		subject,
		text,
	}
	if (html) {
		mailOptions.html = html
	}

	try {
		const info = await transporter.sendMail(mailOptions)
		console.log('Email sent: ', info.response)
		return info
	} catch (error) {
		console.error('Email error:', error)
		throw error
	}
}
